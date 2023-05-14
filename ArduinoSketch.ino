/*
Import library from "https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library/blob/master/README.md" to connect with display
How to read write data from SD card "https://docs.arduino.cc/learn/programming/sd-guide"
*/
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
using namespace std;

#define TFT_DC 53 //should be value of DC pin
#define TFT_CS 10 //should be value of CS pin
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

const float BETA = 3950;

void setup() {
  tft.begin();
  Serial.begin(9600);
  tft.setTextColor(ILI9341_WHITE);
  //tft.setRotation(90); I think the software stops you from dislaying things horizontally
 
}

void loop() {     // During loop..
  int analogValue = analogRead(A0);
  float celsius = 1 / (log(1 / (1023. / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;
  delay(1000);
  tft.setCursor(10, 10);
  tft.setTextSize(3);
  //tft.drawRect(0,0, 240, 70, ILI9341_WHITE);
  tft.println(celsius);
  tft.setCursor(5, 40);
  tft.setTextSize(2);
  tft.println("Degree C");
  delay(1000);
  tft.fillScreen(0x0000);
  delay(1000);

}
