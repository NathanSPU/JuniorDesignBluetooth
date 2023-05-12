#include <ArduinoBLE.h> // Not working with Arduino Mega, other option? Is it needed?

#define CSC_Service_UUID "0x1816"
#define CSC_MEASUREMENT_CHARACTERISTIC_UUID "0x2A5B"

BLEDevice rpmSensor = BLE.available();
int incomingByte;
void setup() {
  Serial.begin(9600);
  pinMode(4, INPUT);
  while (!Serial); // potential cut
}

void loop() {
  if (rpmSensor) {
    incomingByte = Serial.read();
  }

}
