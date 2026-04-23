#include <PFLOW2001.h>

PFLOW2001 flowSensor;

void setup() {
  Serial.begin(115200);
  delay(1000);

  uint8_t detectedAddress;
  if (PFLOW2001::scanKnownAddresses(Wire, detectedAddress)) {
    Serial.print("Found PFLOW2001 candidate at 0x");
    Serial.println(detectedAddress, HEX);
  } else {
    Serial.println("No PFLOW2001 found at 0x50 or 0x28");
  }

  if (!flowSensor.begin(PFLOW2001::DEFAULT_ADDRESS_7BIT, 100000)) {
    Serial.print("Begin failed: ");
    Serial.println(flowSensor.lastErrorString());
    Serial.println("Try PFLOW2001::ALT_ADDRESS_7BIT if needed.");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("PFLOW2001 ready");
  delay(2000);

  String serialNum;
  if (flowSensor.readSerial(serialNum)) {
    Serial.print("Serial: ");
    Serial.println(serialNum);
  } else {
    Serial.print("Serial read failed: ");
    Serial.println(flowSensor.lastErrorString());
  }
}

void loop() {
  float flowSccm = NAN;
  float gasTempC = NAN;

  bool flowOk = flowSensor.readFlow(flowSccm);
  String flowErr = flowOk ? "" : String(flowSensor.lastErrorString());

  delay(10);

  bool gasOk = flowSensor.readGasTemperature(gasTempC);
  String gasErr = gasOk ? "" : String(flowSensor.lastErrorString());

  if (flowOk) {
    Serial.print("Flow: ");
    Serial.print(flowSccm, 3);
    Serial.print(" sccm");
  } else {
    Serial.print("Flow failed: ");
    Serial.print(flowErr);
  }

  Serial.print("    ");

  if (gasOk) {
    Serial.print("Tgas: ");
    Serial.print(gasTempC, 2);
    Serial.print(" C");
  } else {
    Serial.print("Tgas failed: ");
    Serial.print(gasErr);
  }

  Serial.println();
  delay(250);
}