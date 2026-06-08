#include <PFLOW2001.h>

PFLOW2001 flowSensor;

const int N = 20;
const uint16_t SAMPLE_DELAY_MS = 10;

// decrease ALPHA for smoother data, increase Alpha for faster response.
// e.g. .05 = stronger smoothing, 0.2 = quicker response.
const float FLOW_ALPHA = 0.1f;
const float TEMP_ALPHA = 0.1f;

void setup() {
  Serial.begin(115200);
  delay(1000);

  uint8_t foundAddress;
  if (PFLOW2001::scanKnownAddresses(Wire, foundAddress)) {
    Serial.print("Found PFLOW2001 candidate at 0x");
    Serial.println(foundAddress, HEX);
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

  Serial.println("PFLOW2001 filtered flow + EMA gas temperature example");
  delay(2000);
  flowSensor.resetFilter();
}

void loop() {
  float avgFlow = NAN;
  float emaFlow = NAN;
  float gasTempC = NAN;

  static float emaTempC = NAN;

  bool flowOk = flowSensor.readFilteredFlow(
      emaFlow, avgFlow, N, SAMPLE_DELAY_MS, FLOW_ALPHA);
  String flowErr = flowOk ? "" : String(flowSensor.lastErrorString());

  delay(10);

  bool gasOk = flowSensor.readGasTemperature(gasTempC);
  String gasErr = gasOk ? "" : String(flowSensor.lastErrorString());

  if (gasOk) {
    if (isnan(emaTempC)) {
      emaTempC = gasTempC;
    } else {
      emaTempC = TEMP_ALPHA * gasTempC + (1.0f - TEMP_ALPHA) * emaTempC;
    }
  }

  if (flowOk) {
    Serial.print("Avg flow: ");
    Serial.print(avgFlow, 3);
    Serial.print(" sccm    EMA flow: ");
    Serial.print(emaFlow, 3);
    Serial.print(" sccm");
  } else {
    Serial.print("Flow filter failed: ");
    Serial.print(flowErr);
  }

  Serial.print("    ");

  if (gasOk) {
    Serial.print("Tgas: ");
    Serial.print(gasTempC, 2);
    Serial.print(" C    EMA Tgas: ");
    Serial.print(emaTempC, 2);
    Serial.print(" C");
  } else {
    Serial.print("Tgas failed: ");
    Serial.print(gasErr);
  }

  Serial.println();
  delay(250);
}
