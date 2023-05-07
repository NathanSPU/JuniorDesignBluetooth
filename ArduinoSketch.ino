/*

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
