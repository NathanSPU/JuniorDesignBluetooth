/*
Import library from "https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library/blob/master/README.md" to connect with display
How to read write data from SD card "https://docs.arduino.cc/learn/programming/sd-guide"
*/
int ldr = A0;

void setup() {
    pinMode(ldr, INPUT); //initialize ldr sensor as INPUT
    Serial.begin(9600); //begin the serial monitor at 9600 baud
}

void loop() {
    int data=analogRead(ldr);
    Serial.print(“ldr reading=“);
    Serial.println(data);
    delay(1000);
}
