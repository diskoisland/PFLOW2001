#include <Wire.h>
#include <PFLOW2001.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();
  Wire.setClock(100000);

  Serial.println("Scanning PFLOW2001 known addresses...");

  uint8_t foundAddress;
  if (PFLOW2001::scanKnownAddresses(Wire, foundAddress)) {
    Serial.print("Sensor responded at 0x");
    Serial.println(foundAddress, HEX);
  } else {
    Serial.println("No response at 0x50 or 0x28");
  }

  Serial.println("General I2C scan:");
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
    }
  }
}

void loop() {
}
