//dirt cheap wireless RX
void setup()
{
 Serial.begin(2400);
 Serial.println('Init...');
 pinMode(13, OUTPUT);
}

void loop()
{
 // if incoming serial
 if (Serial.available()) {
   readSerial();
   digitalWrite(13, HIGH);
 } else {
   digitalWrite(13, LOW);
 }
 delay(10);
} 

void readSerial() {
 char val = Serial.read();
 Serial.print(val);
}
