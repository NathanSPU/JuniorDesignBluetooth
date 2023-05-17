#include "SPI.h"
#include "SD.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Wire.h"
using namespace std;

#define TFT_DC 53 //should be value of DC pin
#define TFT_CS 10 //should be value of CS pin
#define CS_PIN 53 //should be Sd card value of CS pin
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

File root;

const float BETA = 3950;
const int mpuAddress = 0x68;   // I2C address of the Accelerometer

void setup() {
  Serial.begin(115200);
  tft.begin();
  Wire.begin();
  tft.setTextColor(ILI9341_WHITE);

    // Initialize the MPU-6050 and test if it is connected.
  Wire.beginTransmission( mpuAddress);
  Wire.write( 0x6B);                           // PWR_MGMT_1 register
  Wire.write( 0);                              // set to zero (wakes up the MPU-6050)
  auto error = Wire.endTransmission();
  if( error != 0)
  {
    Serial.println(F( "Error, MPU-6050 not found"));
    for(;;);                                   // halt the sketch if error encountered
  }

  if (!SD.begin(CS_PIN)) {
    Serial.println("Card initialization failed!");
    while (true);
  }

  Serial.println("Files in the card:");
  root = SD.open("/");
  printDirectory(root, 0);
  Serial.println("");

  // Example of reading file from the card:
  File textFile = SD.open("wokwi.txt");
  if (textFile) {
    Serial.print("wokwi.txt: ");
    while (textFile.available()) {
      Serial.write(textFile.read());
    }
    textFile.close();
  } else {
    Serial.println("error opening wokwi.txt!");
  }
}

void loop() {
  Wire.beginTransmission( mpuAddress);
  Wire.write( 0x3B);                   // Starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission( false);        // No stop condition for a repeated start

  int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

  Wire.requestFrom( mpuAddress, 14);   // request a total of 14 bytes
  AcX = Wire.read()<<8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY = Wire.read()<<8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read()<<8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read()<<8 | Wire.read();  // 0x41 (TEMP_OUT_H)   & 0x42 (TEMP_OUT_L)
  GyX = Wire.read()<<8 | Wire.read();  // 0x43 (GYRO_XOUT_H)  & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read()<<8 | Wire.read();  // 0x45 (GYRO_YOUT_H)  & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read()<<8 | Wire.read();  // 0x47 (GYRO_ZOUT_H)  & 0x48 (GYRO_ZOUT_L)

  float FAcX = AcX / 16384*9.8;
  float FAcY = AcX / 16384*9.8;
  float FAcZ = AcX / 16384*9.8;
  float FGyX = AcX / 16384;
  float FGyY = AcX / 16384;
  float FGyZ = AcX / 16384;
  
  int analogValue = analogRead(A0); //change based on what port we are using
  float celsius = 1 / (log(1 / (1023. / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;
  tft.setCursor(10, 10);
  tft.setTextSize(3);
  tft.println(celsius);
  tft.setCursor(5, 40);
  tft.setTextSize(2);
  tft.println("Degree C");

  tft.setCursor(140, 60);
  tft.println(AcX/16384*9.8065); //9.8065
  tft.setCursor(5,60);
  tft.println("Accel in x");

  tft.setCursor(140, 80);
  tft.println(AcY/16384*9.8065); //9.8065
  tft.setCursor(5,80);
  tft.println("Accel in y");

  tft.setCursor(140, 100);
  tft.println(AcZ/16384*9.8065); //9.8065
  tft.setCursor(5,100);
  tft.println("Accel in z");

  tft.setCursor(140, 120);
  tft.println(GyX);
  tft.setCursor(5,120);
  tft.println("Deg/s in x");

  tft.setCursor(140, 140);
  tft.println(GyY);
  tft.setCursor(5,140);
  tft.println("Deg/s in y");

  tft.setCursor(140, 160);
  tft.println(GyZ);
  tft.setCursor(5,160);
  tft.println("Deg/s in z");

  File textFile = SD.open("wokwi.txt");
  String mystring = dtostrf(celsius,9,2,7) + ',' + AcX + ',' + AcY +',' + AcZ + ',' + GyX + ',' + GyY + ',' + GyZ;

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
  
  delay(3000);
  tft.fillScreen(0x0000);
  delay(1000);
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
