#include "SPI.h"
#include "LCDWIKI_GUI.h"
#include "LCDWIKI_SPI.h"
#include "Wire.h"
#include "DallasTemperature.h"
#include "OneWire.h"
using namespace std;

#define MODEL ILI9341
#define CS    A5    
#define CD    A3
#define RST   A4
#define MOSI  51
#define MISO  50
#define SCK   52
#define CLK   34
#define LED   A0   //if you don't need to control the LED pin,you should set it to -1 and set it to 3.3V
#define ONE_WIRE_BUS 15

const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
int c = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LCDWIKI_SPI mylcd(MODEL,CS,CD,MISO,MOSI,RST,SCK,LED); //model,cs,dc,miso,mosi,reset,sck,led

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

  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission

  calculate_IMU_error();
  delay(20);
}

void loop() {
  // === Read acceleromter data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
  // Calculating Roll and Pitch from the accelerometer data
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - 0.58; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) + 1.58; // AccErrorY ~(-1.58)
  // === Read gyroscope data === //
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  // Correct the outputs with the calculated error values
  GyroX = GyroX + 0.56; // GyroErrorX ~(-0.56)
  GyroY = GyroY - 2; // GyroErrorY ~(2)
  GyroZ = GyroZ + 0.79; // GyroErrorZ ~ (-0.8)
  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  gyroAngleX = gyroAngleX + GyroX * elapsedTime; // deg/s * s = deg
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  yaw =  yaw + GyroZ * elapsedTime; //might change this to be Gyro Z
  // Complementary filter - combine acceleromter and gyro angle values
  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;
  
  // Print the values on the serial monitor
  Serial.print(roll);
  Serial.print("/");
  Serial.print(pitch);
  Serial.print("/");
  Serial.println(yaw);
  
  Display();

}
void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gyro data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // Read accelerometer values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    c++;
  }
  //Divide the sum by 200 to get the error value
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;
  c = 0;
  // Read gyro values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  //Divide the sum by 200 to get the error value
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  GyroErrorZ = GyroErrorZ / 200;
  // Print the error values on the Serial Monitor
  Serial.print("AccErrorX: ");
  Serial.println(AccErrorX);
  Serial.print("AccErrorY: ");
  Serial.println(AccErrorY);
  Serial.print("GyroErrorX: ");
  Serial.println(GyroErrorX);
  Serial.print("GyroErrorY: ");
  Serial.println(GyroErrorY);
  Serial.print("GyroErrorZ: ");
  Serial.println(GyroErrorZ);
}

void Display() {
  int numxoffset = 0;
  int strxoffset = 142;
  int yoffset = 10;

  sensors.requestTemperatures();
  float celsius = sensors.getTempCByIndex(0);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(celsius, 2, numxoffset, 5+yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String(" C",strxoffset,5+yoffset);
  mylcd.Draw_Circle(strxoffset+5,5+yoffset,4);
  
  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(AccX, 2, numxoffset,35+yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("AccX m/s^2",strxoffset,40+yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(AccY, 2, numxoffset,65+yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("AccY m/s^2",strxoffset,70+yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(AccZ, 2, numxoffset,95+yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("AccZ m/s^2",strxoffset,100+yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(gyroAngleX, 2, numxoffset,125+yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Deg X",strxoffset,130+yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(gyroAngleY, 2, numxoffset,155+yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Deg Y",strxoffset,160+yoffset);

  mylcd.Set_Text_Size(4);
  mylcd.Print_Number_Float(yaw, 2, numxoffset,185+yoffset, '.', 6, ' ');
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Deg Z",strxoffset,190+yoffset);
  
  delay(2000);
  mylcd.Fill_Screen(0xFFFF);

  mylcd.Set_Text_Size(8);
  mylcd.Print_Number_Float(yaw, 2, 0,15, '.', 6, ' ');
  mylcd.Print_String("m/s",120,100);

  delay(2000);
  mylcd.Fill_Screen(0xFFFF);

}
