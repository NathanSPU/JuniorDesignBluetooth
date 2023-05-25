#include "SPI.h"
#include "LCDWIKI_GUI.h"
#include "LCDWIKI_SPI.h"
#include "Wire.h"
#include "DallasTemperature.h"
#include "OneWire.h"
using namespace std;

#define MODEL ILI9341
#define CS A5
#define CD A3
#define RST A4
#define MOSI 51
#define MISO 50
#define SCK 52
#define CLK 34
#define LED A0  //if you don't need to control the LED pin,you should set it to -1 and set it to 3.3V
#define ONE_WIRE_BUS 15

#define MPU6050_ACCEL_XOUT_H       0x3B   // R  
#define MPU6050_ACCEL_XOUT_L       0x3C   // R  
#define MPU6050_ACCEL_YOUT_H       0x3D   // R  
#define MPU6050_ACCEL_YOUT_L       0x3E   // R  
#define MPU6050_ACCEL_ZOUT_H       0x3F   // R  
#define MPU6050_ACCEL_ZOUT_L       0x40   // R  
#define MPU6050_TEMP_OUT_H         0x41   // R  
#define MPU6050_TEMP_OUT_L         0x42   // R  
#define MPU6050_GYRO_XOUT_H        0x43   // R  
#define MPU6050_GYRO_XOUT_L        0x44   // R  
#define MPU6050_GYRO_YOUT_H        0x45   // R  
#define MPU6050_GYRO_YOUT_L        0x46   // R  
#define MPU6050_GYRO_ZOUT_H        0x47   // R  
#define MPU6050_GYRO_ZOUT_L        0x48   // R  
#define MPU6050_WHO_AM_I           0x75   // R
#define MPU6050_PWR_MGMT_1         0x6B   // R/W
#define MPU6050_PWR_MGMT_2         0x6C   // R/
#define MPU6050_I2C_ADDRESS 0x68

const int MPU = 0x68;  // MPU6050 I2C address
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
int c = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LCDWIKI_SPI mylcd(MODEL, CS, CD, MISO, MOSI, RST, SCK, LED);  //model,cs,dc,miso,mosi,reset,sck,led

// Declaring an union for the registers and the axis values.
// The byte order does not match the byte order of 
// the compiler and AVR chip.
// The AVR chip (on the Arduino board) has the Low Byte 
// at the lower address.
// But the MPU-6050 has a different order: High Byte at
// lower address, so that has to be corrected.
// The register part "reg" is only used internally, 
// and are swapped in code.
typedef union accel_t_gyro_union
{
  struct
  {
    uint8_t x_accel_h;
    uint8_t x_accel_l;
    uint8_t y_accel_h;
    uint8_t y_accel_l;
    uint8_t z_accel_h;
    uint8_t z_accel_l;
    uint8_t t_h;
    uint8_t t_l;
    uint8_t x_gyro_h;
    uint8_t x_gyro_l;
    uint8_t y_gyro_h;
    uint8_t y_gyro_l;
    uint8_t z_gyro_h;
    uint8_t z_gyro_l;
  } reg;
  struct 
  {
    int16_t x_accel;
    int16_t y_accel;
    int16_t z_accel;
    int16_t temperature;
    int16_t x_gyro;
    int16_t y_gyro;
    int16_t z_gyro;
  } value;
};


void setup() {
  Serial.begin(9600);
  mylcd.Init_LCD();
  Wire.begin();
  sensors.begin();
  mylcd.Fill_Screen(0xFFFF);
  mylcd.Set_Text_Back_colour(0xFFFF);
  mylcd.Set_Draw_color(0x0000);
  mylcd.Set_Text_colour(0x0000);
  mylcd.Set_Text_Size(4);
  mylcd.Set_Rotation(3);

  int error;
  uint8_t c;


  Serial.begin(9600);
  Serial.println(F("InvenSense MPU-6050"));
  Serial.println(F("June 2012"));

  // Initialize the 'Wire' class for the I2C-bus.
  Wire.begin();


  // default at power-up:
  //    Gyro at 250 degrees second
  //    Acceleration at 2g
  //    Clock source at internal 8MHz
  //    The device is in sleep mode.
  //

  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  Serial.print(F("WHO_AM_I : "));
  Serial.print(c,HEX);
  Serial.print(F(", error = "));
  Serial.println(error,DEC);

  // According to the datasheet, the 'sleep' bit
  // should read a '1'.
  // That bit has to be cleared, since the sensor
  // is in sleep mode at power-up. 
  error = MPU6050_read (MPU6050_PWR_MGMT_1, &c, 1);
  Serial.print(F("PWR_MGMT_1 : "));
  Serial.print(c,HEX);
  Serial.print(F(", error = "));
  Serial.println(error,DEC);


  // Clear the 'sleep' bit to start the sensor.
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
}

void loop() { //gathers data in 0.1 second unless you do something like try to display it to slow it down
  byte error, address;
  double dT;
  accel_t_gyro_union accel_t_gyro;
  previousTime = currentTime;                         // Previous time is stored before the actual time read
  currentTime = millis();                             // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000;  // Divide by 1000 to get seconds

  // Read the raw values.
  // Read 14 bytes at once, 
  // containing acceleration, temperature and gyro.
  // With the default settings of the MPU-6050,
  // there is no filter enabled, and the values
  // are not very stable.
  error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &accel_t_gyro, sizeof(accel_t_gyro));
  Serial.print(F("Read accel, temp and gyro, error = "));
  Serial.println(error,DEC);

  // Swap all high and low bytes.
  // After this, the registers values are swapped, 
  // so the structure name like x_accel_l does no 
  // longer contain the lower byte.
  uint8_t swap;
  #define SWAP(x,y) swap = x; x = y; y = swap

  SWAP (accel_t_gyro.reg.x_accel_h, accel_t_gyro.reg.x_accel_l);
  SWAP (accel_t_gyro.reg.y_accel_h, accel_t_gyro.reg.y_accel_l);
  SWAP (accel_t_gyro.reg.z_accel_h, accel_t_gyro.reg.z_accel_l);
  SWAP (accel_t_gyro.reg.t_h, accel_t_gyro.reg.t_l);
  SWAP (accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
  SWAP (accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);
  SWAP (accel_t_gyro.reg.z_gyro_h, accel_t_gyro.reg.z_gyro_l);

  // Print the raw acceleration values

  Serial.print(F("accel x,y,z: "));
  Serial.print(accel_t_gyro.value.x_accel / 16384.0, DEC);
  Serial.print(F(", "));
  Serial.print(accel_t_gyro.value.y_accel / 16384.0, DEC);
  Serial.print(F(", "));
  Serial.print(accel_t_gyro.value.z_accel / 16384.0, DEC);
  Serial.println(F(""));

  // The temperature sensor is -40 to +85 degrees Celsius.
  // It is a signed integer.
  // According to the datasheet: 
  //   340 per degrees Celsius, -512 at 35 degrees.
  // At 0 degrees: -512 - (340 * 35) = -12412

  Serial.print(F("temperature: "));
  dT = ( (double) accel_t_gyro.value.temperature + 12412.0) / 340.0;
  Serial.print(dT, 3);
  Serial.print(F(" degrees Celsius"));
  Serial.println(F(""));


  // Print the raw gyro values.

  Serial.print(F("gyro x,y,z : "));
  Serial.print(accel_t_gyro.value.x_gyro/ 131.0, DEC);
  Serial.print(F(", "));
  Serial.print(accel_t_gyro.value.y_gyro/ 131.0, DEC);
  Serial.print(F(", "));
  Serial.print(accel_t_gyro.value.z_gyro/ 131.0, DEC);
  Serial.print(F(", "));
  Serial.println(F(""));
  Serial.print(F("time elapsed: "));
  Serial.println(elapsedTime);

  sensors.requestTemperatures();
  float celsius = sensors.getTempCByIndex(0);
  Display(celsius,accel_t_gyro.value.x_accel, accel_t_gyro.value.y_accel, accel_t_gyro.value.z_accel, accel_t_gyro.value.x_gyro, accel_t_gyro.value.y_gyro, accel_t_gyro.value.z_gyro, accel_t_gyro.value.z_gyro);
  Serial.print(F("time elapsed: "));
  Serial.println(elapsedTime);
}
  
int MPU6050_read(int start, uint8_t *buffer, int size)
{
  int i, n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
    return (-10);

  n = Wire.endTransmission(false);    // hold the I2C-bus
  if (n != 0)
    return (n);

  // Third parameter is true: relase I2C-bus after data is read.
  Wire.requestFrom(MPU6050_I2C_ADDRESS, size, true);
  i = 0;
  while(Wire.available() && i<size)
  {
    buffer[i++]=Wire.read();
  }
  if ( i != size)
    return (-11);

  return (0);  // return : no error
}


// --------------------------------------------------------
// MPU6050_write
//
// This is a common function to write multiple bytes to an I2C device.
//
// If only a single register is written,
// use the function MPU_6050_write_reg().
//
// Parameters:
//   start : Start address, use a define for the register
//   pData : A pointer to the data to write.
//   size  : The number of bytes to write.
//
// If only a single register is written, a pointer
// to the data has to be used, and the size is
// a single byte:
//   int data = 0;        // the data to write
//   MPU6050_write (MPU6050_PWR_MGMT_1, &c, 1);
//
int MPU6050_write(int start, const uint8_t *pData, int size)
{
  int n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);        // write the start address
  if (n != 1)
    return (-20);

  n = Wire.write(pData, size);  // write data bytes
  if (n != size)
    return (-21);

  error = Wire.endTransmission(true); // release the I2C-bus
  if (error != 0)
    return (error);

  return (0);         // return : no error
}

// --------------------------------------------------------
// MPU6050_write_reg
//
// An extra function to write a single register.
// It is just a wrapper around the MPU_6050_write()
// function, and it is only a convenient function
// to make it easier to write a single register.
//
int MPU6050_write_reg(int reg, uint8_t data)
{
  int error;

  error = MPU6050_write(reg, &data, 1);

  return (error);
}

float Display(float celsius,float AccelX,float AccelY,float AccelZ,float GyroX,float GyroY,float GyroZ,float Speed) {
  int numxoffset = 0;
  int strxoffset = 142;
  int yoffset = 10;

  mylcd.Fill_Screen(0xFFFF);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(celsius, 2, numxoffset, 5 + yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String(" C", strxoffset, 5 + yoffset);
  mylcd.Draw_Circle(strxoffset + 5, 5 + yoffset, 4);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(AccelX, 2, numxoffset, 35 + yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("AccX m/s^2", strxoffset, 40 + yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(AccelY, 2, numxoffset, 65 + yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("AccY m/s^2", strxoffset, 70 + yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(AccelZ, 2, numxoffset, 95 + yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("AccZ m/s^2", strxoffset, 100 + yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(GyroX, 2, numxoffset, 125 + yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Deg X", strxoffset, 130 + yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(GyroY, 2, numxoffset, 155 + yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Deg Y", strxoffset, 160 + yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(GyroZ, 2, numxoffset, 185 + yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Deg Z", strxoffset, 190 + yoffset);

  delay(2000);
  mylcd.Fill_Screen(0xFFFF);

  mylcd.Set_Text_Size(8);
  mylcd.Print_Number_Float(GyroZ, 2, 0, 15, '.', 6, ' ');
  mylcd.Print_String("m/s", 120, 100);
}
