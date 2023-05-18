#include "SPI.h"
#include "SD.h"
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
#define LED   A0   //if you don't need to control the LED pin,you should set it to -1 and set it to 3.3V
#define ONE_WIRE_BUS 15

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LCDWIKI_SPI mylcd(MODEL,CS,CD,MISO,MOSI,RST,SCK,LED); //model,cs,dc,miso,mosi,reset,sck,led

File root;
const float BETA = 3950;
const int mpuAddress = 0x68;   // I2C address of the Accelerometer

void setup() {
  Serial.begin(115200);
  mylcd.Init_LCD();
  Wire.begin();
  sensors.begin();
  mylcd.Fill_Screen(0xFFFF);
  mylcd.Set_Text_Back_colour(0xFFFF);
  mylcd.Set_Draw_color(0x0000);
  mylcd.Set_Text_colour(0x0000);
  mylcd.Set_Text_Size(4);
  mylcd.Set_Rotation(3);

    // Initialize the MPU-6050 and test if it is connected.
  // Wire.beginTransmission( mpuAddress);
  // Wire.write( 0x6B);                           // PWR_MGMT_1 register
  // Wire.write( 0);                              // set to zero (wakes up the MPU-6050)
  // auto error = Wire.endTransmission();
  // if( error != 0)
  // {
  //   Serial.println(F( "Error, MPU-6050 not found"));
  //   for(;;);                                   // halt the sketch if error encountered
  // }

  // if (!SD.begin(CS_PIN)) {
  //   Serial.println("Card initialization failed!");
  //   while (true);
  // }

  // Serial.println("Files in the card:");
  // root = SD.open("/");
  // printDirectory(root, 0);
  // Serial.println("");

  // // Example of reading file from the card:
  // File textFile = SD.open("wokwi.txt");
  // if (textFile) {
  //   Serial.print("wokwi.txt: ");
  //   while (textFile.available()) {
  //     Serial.write(textFile.read());
  //   }
  //   textFile.close();
  // } else {
  //   Serial.println("error opening wokwi.txt!");
  // }
}

void loop() {
  // Wire.beginTransmission( mpuAddress);
  // Wire.write( 0x3B);                   // Starting with register 0x3B (ACCEL_XOUT_H)
  // Wire.endTransmission( false);        // No stop condition for a repeated start

  int AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ = 1;

  // Wire.requestFrom( mpuAddress, 14);   // request a total of 14 bytes
  // AcX = Wire.read()<<8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  // AcY = Wire.read()<<8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  // AcZ = Wire.read()<<8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  // Tmp = Wire.read()<<8 | Wire.read();  // 0x41 (TEMP_OUT_H)   & 0x42 (TEMP_OUT_L)
  // GyX = Wire.read()<<8 | Wire.read();  // 0x43 (GYRO_XOUT_H)  & 0x44 (GYRO_XOUT_L)
  // GyY = Wire.read()<<8 | Wire.read();  // 0x45 (GYRO_YOUT_H)  & 0x46 (GYRO_YOUT_L)
  // GyZ = Wire.read()<<8 | Wire.read();  // 0x47 (GYRO_ZOUT_H)  & 0x48 (GYRO_ZOUT_L)

  // float FAcX = AcX / 16384*9.8;
  // float FAcY = AcX / 16384*9.8;
  // float FAcZ = AcX / 16384*9.8;
  // float FGyX = AcX / 16384;
  // float FGyY = AcX / 16384;
  // float FGyZ = AcX / 16384;
  
  // int analogValue = analogRead(A0); //change based on what port we are using
  sensors.requestTemperatures();
  float celsius = sensors.getTempCByIndex(0);
  char buffer [7];
  int numxoffset = 0;
  int strxoffset = 142;
  int yoffset = 10;
  // String mystring = dtostrf(celsius,9,2,buffer) + " °C";
  mylcd.Set_Text_Size(4);

  mylcd.Print_Number_Float(celsius, 2, numxoffset, 5+yoffset, '.', 6, ' ');

  // mystring = AcX + "m/s^2 in x";
  mylcd.Print_Number_Float(AcX, 2, numxoffset,35+yoffset, '.', 6, ' ');

  // mystring = AcY + "m/s^2 in y";
  mylcd.Print_Number_Float(AcY, 2, numxoffset,65+yoffset, '.', 6, ' ');

 // mystring = AcZ + "m/s^2 in z";
  mylcd.Print_Number_Float(AcZ, 2, numxoffset,95+yoffset, '.', 6, ' ');

 // mystring = GyX + "Deg/s in x";
  mylcd.Print_Number_Float(GyX, 2, numxoffset,125+yoffset, '.', 6, ' ');

//  mystring = GyY + "Deg/s in y";
  mylcd.Print_Number_Float(GyY, 2, numxoffset,155+yoffset, '.', 6, ' ');
  
 // mystring = GyZ + "Deg/s in z";
  mylcd.Print_Number_Float(GyZ, 2, numxoffset,185+yoffset, '.', 6, ' ');
  
  mylcd.Set_Text_Size(3);

  mylcd.Print_String(" C",strxoffset,5+yoffset);
  mylcd.Draw_Circle(strxoffset+5,5+yoffset,4);
  mylcd.Print_String("AccX m/s^2",strxoffset,40+yoffset);
  mylcd.Print_String("AccY m/s^2",strxoffset,70+yoffset);
  mylcd.Print_String("AccZ m/s^2",strxoffset,100+yoffset);
  mylcd.Print_String("Deg X",strxoffset,130+yoffset);
  mylcd.Print_String("Deg Y",strxoffset,160+yoffset);
  mylcd.Print_String("Deg Z",strxoffset,190+yoffset);
  
  // File textFile = SD.open("wokwi.txt");
  // String mystring = dtostrf(celsius,9,2,7) + ',' + AcX + ',' + AcY +',' + AcZ + ',' + GyX + ',' + GyY + ',' + GyZ;

  // if (textFile) {
  //   textFile.println(mystring);
  //   textFile.close();
  //   //dtostrf(celsius,9,2,space) +',' + dtostrf(FAcX,9,2,space) + ','+ dtostrf(FAcY,9,2,space) + ',' + dtostrf(FAcZ,9,2,space) +',' + dtostrf(FGyX,9,2,space) +',' + dtostrf(FGyY,9,2,space) + ',' + dtostrf(FGyZ,9,2,space) +',' 
  // }
  // File writtenFile = SD.open("wokwi.txt");
  // if (writtenFile) {
  //   Serial.print("wokwi.txt: ");
  //   while (writtenFile.available()) {
  //   Serial.write(writtenFile.read());
  //   }
  // writtenFile.close();
  // } else {
  //   Serial.println("error opening wokwi.txt!");
  // }
  
  delay(2000);
  mylcd.Fill_Screen(0xFFFF);

  mylcd.Set_Text_Size(8);
  mylcd.Print_Number_Float(GyZ, 2, 0,15, '.', 6, ' ');
  mylcd.Print_String("m/s",120,100);

  delay(2000);
  mylcd.Fill_Screen(0xFFFF);
}
