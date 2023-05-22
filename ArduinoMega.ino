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
}

void loop() {
  int AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ = 1;
  
  sensors.requestTemperatures();
  float celsius = sensors.getTempCByIndex(0);

  int numxoffset = 0;
  int strxoffset = 142;
  int yoffset = 10;

  mylcd.Set_Text_Size(4);

  mylcd.Print_Number_Float(celsius, 2, numxoffset, 5+yoffset, '.', 6, ' ');

  mylcd.Print_Number_Float(AcX, 2, numxoffset,35+yoffset, '.', 6, ' ');

  mylcd.Print_Number_Float(AcY, 2, numxoffset,65+yoffset, '.', 6, ' ');

  mylcd.Print_Number_Float(AcZ, 2, numxoffset,95+yoffset, '.', 6, ' ');

  mylcd.Print_Number_Float(GyX, 2, numxoffset,125+yoffset, '.', 6, ' ');

  mylcd.Print_Number_Float(GyY, 2, numxoffset,155+yoffset, '.', 6, ' ');
  
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
  
  delay(2000);
  mylcd.Fill_Screen(0xFFFF);

  mylcd.Set_Text_Size(8);
  mylcd.Print_Number_Float(GyZ, 2, 0,15, '.', 6, ' ');
  mylcd.Print_String("m/s",120,100);

  delay(2000);
  mylcd.Fill_Screen(0xFFFF);
}
