#include <SoftwareSerial.h>

SoftwareSerial wifiSerial(2, 3);      // RX, TX for ESP8266

void setup() {
  Serial.begin(115200);
  wifiSerial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(wifiSerial.available())
    Serial.write(wifiSerial.read());
  if(Serial.available())
    wifiSerial.write(Serial.read());
}
