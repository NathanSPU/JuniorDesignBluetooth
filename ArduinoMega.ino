#include "SPI.h"
#include "LCDWIKI_GUI.h"
#include "LCDWIKI_SPI.h"
#include "Wire.h"
#include "DallasTemperature.h"
#include "Adafruit_MPU6050.h"
#include "Adafruit_Sensor.h"
#include "OneWire.h"
using namespace std;

//define all the important ports that we might need to reference
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

//keep track of time so we can derive the radial position and velociy from accelerometer
float elapsedTime, currentTime, previousTime;

//global variables for the offset variables 
float axoffset, ayoffset, azoffset;
float gxoffset, gyoffset, gzoffset;

//global variables for radial position calculation
float RadposX = 0;
float RadposY = 0;
float RadposZ = 0;

//global variable for vehicle speed
float Speedaccel = 0;

//set-up for accelerometer
Adafruit_MPU6050 mpu;

//set-up for temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//set-up for the display
LCDWIKI_SPI mylcd(MODEL, CS, CD, MISO, MOSI, RST, SCK, LED);  //model,cs,dc,miso,mosi,reset,sck,led
bool displayMode = false;
void setup() {

  //start all the communication among the various sensor and microcontrollers
  Serial.begin(9600);
  Serial2.begin(9600);
  mylcd.Init_LCD();
  Wire.begin();
  sensors.begin();

  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

   // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  if (displayMode) {
  //clear the screen, set text-size, color, and direction
    Serial.println("Setting up display");
    mylcd.Fill_Screen(0xFFFF);
    mylcd.Set_Text_Back_colour(0xFFFF);
    mylcd.Set_Draw_color(0x0000);
    mylcd.Set_Text_colour(0x0000);
    mylcd.Set_Text_Size(4);
    mylcd.Set_Rotation(3);
    Serial.println("Display setup complete.");
  }
  else {
    mylcd.Fill_Screen(0x0000);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);

  //calibrate the accelerometer this gives us the offset values to remove some of the error this is done by averaging 200 values
  for (int i = 0; i <= 200; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    axoffset += a.acceleration.x;
    ayoffset += a.acceleration.y;
    azoffset += a.acceleration.z;
    gxoffset += g.gyro.x;
    gyoffset += g.gyro.y;
    gzoffset += g.gyro.z;

  }
  Serial.println(axoffset);
  Serial.println(ayoffset);
  Serial.println(azoffset);
  axoffset = axoffset/200;
  ayoffset = ayoffset/200;
  azoffset = azoffset/200;
  Serial.println(axoffset);
  Serial.println(ayoffset);
  Serial.println(azoffset);

  gxoffset = gxoffset/200;
  gyoffset = gyoffset/200;
  gzoffset = gzoffset/200;

  Serial.println("Calibration finished");
  if (displayMode) {
    mylcd.Set_Text_Size(3);
    mylcd.Print_String(" m/s", 142, 15);
    mylcd.Print_String("AccX m/s^2", 142, 50);
    mylcd.Print_String("AccY m/s^2", 142, 80);
    mylcd.Print_String("AccZ m/s^2", 142, 110);
    mylcd.Print_String("Deg X", 152, 140);
    mylcd.Print_String("Deg Y", 152, 170);
    mylcd.Print_String("Deg Z", 152, 200);
    mylcd.Draw_Circle(297, 215, 2);
    mylcd.Set_Text_Size(2);
    mylcd.Print_String("C",300, 220);
    mylcd.Set_Text_Size(4);
    Serial.println("Starting loop!");
  }
}

void loop() { //gathers data in 0.1 second unless you do something like try to display it to slow it down
  previousTime = currentTime;                         // Previous time is stored before the actual time read

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  currentTime = millis();                             // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000;  // Divide by 1000 to get seconds
  float currentTimeWrite = currentTime / 1000;

  //Collect all of the data

  //Sunfounder temperature sensor
  sensors.requestTemperatures();
  float tempSun = sensors.getTempCByIndex(0);

  //Accelerometer temperature sensor
  float tempAccel = temp.temperature;

  float tempAvg = ((tempSun + tempAccel)/2) - 1;

  //Accelerometer gyroscope data
  float gyroX = ((g.gyro.x-gxoffset)*(180/M_PI))*elapsedTime;
  float gyroY = ((g.gyro.y-gyoffset)*(180/M_PI))*elapsedTime;
  float gyroZ = ((g.gyro.z-gzoffset)*(180/M_PI))*elapsedTime;

  delay(500);
  //Accelerometer acceleration data

  float AccelX = a.acceleration.x-axoffset;
  float AccelY = a.acceleration.y-ayoffset;
  float AccelZ = a.acceleration.z-azoffset;

  //calulate vehicle speed and radial position,

  RadposX += gyroX; //converted to degrees
  RadposY += gyroY;
  RadposZ += gyroZ;
  Speedaccel = AccelY * elapsedTime;

  //Save to SD card
  char buffer[7];
  char buffer1[7];
  char buffer2[7];
  char buffer3[7];
  char buffer4[7];
  char buffer5[7];
  char buffer6[7];
  char buffer7[7];

  Serial2.write(dtostrf(Speedaccel, 4, 2, buffer));
  Serial2.write(", ");
  
  Serial2.write(dtostrf(AccelX, 4, 2, buffer1));
  Serial2.write(", ");
  
  Serial2.write(dtostrf(AccelY, 4, 2, buffer2));
  Serial2.write(", ");
  
  Serial2.write(dtostrf(AccelZ, 4, 2, buffer3));
  Serial2.write(", ");
  
  Serial2.write(dtostrf(RadposX, 4, 2, buffer4));
  Serial2.write(", ");
  
  Serial2.write(dtostrf(RadposY, 4, 2, buffer5));
  Serial2.write(", ");
  
  Serial2.write(dtostrf(RadposZ, 4, 2, buffer6));
  Serial2.write(", ");
  
  Serial2.write(dtostrf(tempAvg, 4, 2, buffer7));
  Serial2.println(", ");
  
  Serial2.write(dtostrf(currentTimeWrite, 4, 2, buffer7));
  Serial2.println();
  if (displayMode) {
  delay(4000);

  //display
  Display(tempAvg,AccelX,AccelY,AccelZ,RadposX,RadposY,RadposZ,Speedaccel);
  }

  /* Print out the values */
  // Serial.print("Acceleration X: ");
  // Serial.print(AccelX);
  // Serial.print(", Y: ");
  // Serial.print(AccelY);
  // Serial.print(", Z: ");
  // Serial.print(AccelZ);
  // Serial.println(" m/s^2");

  // Serial.print("Instant X: ");
  // Serial.print(gyroX);
  // Serial.print(", Y: ");
  // Serial.print(gyroY);
  // Serial.print(", Z: ");
  // Serial.print(gyroZ);
  // Serial.println(" deg");

  Serial.print("Rotation X: ");
  Serial.print(RadposX);
  Serial.print(", Y: ");
  Serial.print(RadposY);
  Serial.print(", Z: ");
  Serial.print(RadposZ);
  Serial.println(" deg");
  Serial.println(currentTime/1000);

  // Serial.print("Temp: ");
  // Serial.print(tempAccel);
  // Serial.println(" Degree C");

  // Serial.println("");
  // delay(500);
}

float Display(float tempAvg,float AccelX,float AccelY,float AccelZ,float GyroX,float GyroY,float GyroZ,float Speedaccel) {
  int yoffset = 10;
  if (tempAvg > 99.99) {
    tempAvg = 99.99;
  } else if (tempAvg < -99.99) {
    tempAvg = -99.99;
  }
  if (AccelX > 99.99) {
    AccelX = 99.99;
  } else if (AccelX < -99.99) {
    AccelX = -99.99;
  }
  if (AccelY > 99.99) {
    AccelY = 99.99;
  } else if (AccelY < -99.99) {
    AccelY = -99.99;
  }
  if (AccelZ > 99.99) {
    AccelZ = 99.99;
  } else if (AccelZ < -99.99) {
    AccelZ = -99.99;
  }
  if (GyroX > 99.99) {
    GyroX = 99.99;
  } else if (GyroX < -99.99) {
    GyroX = -99.99;
  }
  if (GyroY > 99.99) {
    GyroY = 99.99;
  } else if (GyroY < -99.99) {
    GyroY = -99.99;
  }
  if (GyroZ > 99.99) {
    GyroZ = 99.99;
  } else if (GyroZ < -99.99) {
    GyroZ = -99.99;
  }
  if (Speedaccel > 99.99) {
    Speedaccel = 99.99;
  } else if (Speedaccel <-99.99) {
    Speedaccel = -99.99;
  }

  mylcd.Print_Number_Float(Speedaccel, 1, 1, 5 + yoffset, '.', 3, ' ');
  mylcd.Print_Number_Float(AccelX, 1, 1, 35 + yoffset, '.', 3, ' ');
  mylcd.Print_Number_Float(AccelY, 1, 1, 65 + yoffset, '.', 3, ' ');
  mylcd.Print_Number_Float(AccelZ, 1, 1, 95 + yoffset, '.', 3, ' ');
  mylcd.Print_Number_Float(GyroX, 1, 1, 125 + yoffset, '.', 3, ' ');
  mylcd.Print_Number_Float(GyroY, 1, 1, 155 + yoffset, '.', 3, ' ');
  mylcd.Print_Number_Float(GyroZ, 1, 1, 185 + yoffset, '.', 3, ' ');
  mylcd.Set_Text_Size(2);
  if (tempAvg > 80) {
    mylcd.Set_Text_colour(0xF800);
    mylcd.Fill_Screen(0xFFFF);
    mylcd.Print_String(" Warning Overheat", 142, 15);
    mylcd.Print_Number_Float(tempAvg, 1, 142, 50, '.', 4, ' ');
    mylcd.Fill_Screen(0xFFFF);
  }
  else {
    mylcd.Set_Text_colour(0x0000);
  }
  mylcd.Print_Number_Float(tempAvg, 1, 250, 220, '.', 4, ' ');
  mylcd.Set_Text_Size(4);
}
