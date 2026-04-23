#include <PFLOW2001.h>

PFLOW2001 flowSensor;

const int N = 20;
const uint16_t SAMPLE_DELAY_MS = 10;
const float ALPHA = 0.1f;

void setup() {
  Serial.begin(115200);
  delay(1000);

  uint8_t foundAddress;
  if (PFLOW2001::scanKnownAddresses(Wire, foundAddress)) {
    Serial.print("Found PFLOW2001 candidate at 0x");
    Serial.println(foundAddress, HEX);
  }

  if (!flowSensor.begin(PFLOW2001::DEFAULT_ADDRESS_7BIT, 100000)) {
    Serial.print("Begin failed: ");
    Serial.println(flowSensor.lastErrorString());
    Serial.println("Try PFLOW2001::ALT_ADDRESS_7BIT if needed.");
    while (1) { delay(1000); }
  }

  Serial.println("PFLOW2001 filtered flow example");
  delay(2000);
  flowSensor.resetFilter();
}

void loop() {
  float avgFlow = NAN;
  float emaFlow = NAN;

  if (flowSensor.readFilteredFlow(emaFlow, avgFlow, N, SAMPLE_DELAY_MS, ALPHA)) {
    Serial.print("Avg flow: ");
    Serial.print(avgFlow, 3);
    Serial.print(" sccm    EMA: ");
    Serial.print(emaFlow, 3);
    Serial.println(" sccm");
  } else {
    Serial.print("No valid samples / read failed: ");
    Serial.println(flowSensor.lastErrorString());
  }

  delay(250);
}
