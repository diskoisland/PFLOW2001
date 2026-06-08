#include "PFLOW2001.h"

PFLOW2001::PFLOW2001(TwoWire& wirePort)
: _wire(&wirePort), _address(DEFAULT_ADDRESS_7BIT), _lastError(OK), _emaFlow(NAN) {}

bool PFLOW2001::begin(uint8_t address, uint32_t clockHz) {
  _address = address;
  _wire->begin();
  _wire->setClock(clockHz);
  return ping();
}

bool PFLOW2001::ping() {
  _wire->beginTransmission(_address);
  if (_wire->endTransmission() == 0) {
    clearError();
    return true;
  }
  setError(ERR_NO_ACK);
  return false;
}

uint8_t PFLOW2001::getAddress() const {
  return _address;
}

PFLOW2001::ErrorCode PFLOW2001::lastError() const {
  return _lastError;
}

const char* PFLOW2001::lastErrorString() const {
  switch (_lastError) {
    case OK: return "OK";
    case ERR_I2C_WRITE_COMMAND: return "I2C command write failed";
    case ERR_I2C_REQUEST: return "I2C requestFrom failed";
    case ERR_I2C_READ_LENGTH: return "I2C read length mismatch or missing bytes";
    case ERR_CRC_WORD1: return "CRC mismatch on first data word";
    case ERR_CRC_WORD2: return "CRC mismatch on second data word";
    case ERR_CRC_SERIAL_BLOCK: return "CRC mismatch in serial number block";
    case ERR_CRC_TEMPERATURE: return "CRC mismatch in temperature data";
    case ERR_TEMPERATURE_NOT_AVAILABLE: return "Temperature value not available";
    case ERR_NO_ACK: return "Sensor did not acknowledge on I2C";
    case ERR_INVALID_ARGUMENT: return "Invalid argument";
    default: return "Unknown error";
  }
}

void PFLOW2001::clearError() {
  _lastError = OK;
}

void PFLOW2001::setError(ErrorCode err) {
  _lastError = err;
}

void PFLOW2001::resetFilter() {
  _emaFlow = NAN;
}

uint8_t PFLOW2001::crc8(const uint8_t* data, size_t len) {
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; b++) {
      if (crc & 0x80) {
        crc = (uint8_t)((crc << 1) ^ 0x07);
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

bool PFLOW2001::scanKnownAddresses(TwoWire& wirePort, uint8_t& foundAddress) {
  const uint8_t candidates[] = { DEFAULT_ADDRESS_7BIT, ALT_ADDRESS_7BIT };
  wirePort.begin();

  for (uint8_t i = 0; i < sizeof(candidates); i++) {
    wirePort.beginTransmission(candidates[i]);
    if (wirePort.endTransmission() == 0) {
      foundAddress = candidates[i];
      return true;
    }
  }
  return false;
}

bool PFLOW2001::writeCommandNoStop(uint16_t cmd) {
  _wire->beginTransmission(_address);
  _wire->write((uint8_t)(cmd >> 8));
  _wire->write((uint8_t)(cmd & 0xFF));

  if (_wire->endTransmission(false) == 0) {
    clearError();
    return true;
  }

  setError(ERR_I2C_WRITE_COMMAND);
  return false;
}

bool PFLOW2001::readBytes(uint16_t cmd, uint8_t* buf, size_t len) {
  if (buf == nullptr || len == 0) {
    setError(ERR_INVALID_ARGUMENT);
    return false;
  }

  if (!writeCommandNoStop(cmd)) {
    return false;
  }

  size_t received = _wire->requestFrom((int)_address, (int)len, (int)true);
  if (received == 0) {
    setError(ERR_I2C_REQUEST);
    return false;
  }
  if (received != len) {
    setError(ERR_I2C_READ_LENGTH);
    return false;
  }

  for (size_t i = 0; i < len; i++) {
    if (!_wire->available()) {
      setError(ERR_I2C_READ_LENGTH);
      return false;
    }
    buf[i] = _wire->read();
  }

  clearError();
  return true;
}

bool PFLOW2001::writeDataWord(uint16_t cmd, uint16_t value) {
  uint8_t data[2];
  data[0] = (uint8_t)(value >> 8);
  data[1] = (uint8_t)(value & 0xFF);
  uint8_t crc = crc8(data, 2);

  _wire->beginTransmission(_address);
  _wire->write((uint8_t)(cmd >> 8));
  _wire->write((uint8_t)(cmd & 0xFF));
  _wire->write(data[0]);
  _wire->write(data[1]);
  _wire->write(crc);

  if (_wire->endTransmission(true) == 0) {
    clearError();
    return true;
  }

  setError(ERR_I2C_WRITE_COMMAND);
  return false;
}

bool PFLOW2001::readFlow(float& flowSccm) {
  uint8_t rx[6];

  if (!readBytes(CMD_READ_FLOW, rx, sizeof(rx))) {
    return false;
  }

  if (crc8(&rx[0], 2) != rx[2]) {
    setError(ERR_CRC_WORD1);
    return false;
  }

  if (crc8(&rx[3], 2) != rx[5]) {
    setError(ERR_CRC_WORD2);
    return false;
  }

  uint32_t raw =
      ((uint32_t)rx[0] << 24) |
      ((uint32_t)rx[1] << 16) |
      ((uint32_t)rx[3] << 8)  |
      ((uint32_t)rx[4]);

  int32_t signedRaw = (int32_t)raw;
  flowSccm = signedRaw / 1000.0f;
  clearError();
  return true;
}

bool PFLOW2001::readTemperatureCommand(uint16_t cmd, float& tempC) {
  uint8_t rx[3];

  if (!readBytes(cmd, rx, sizeof(rx))) {
    return false;
  }

  if (crc8(&rx[0], 2) != rx[2]) {
    setError(ERR_CRC_TEMPERATURE);
    return false;
  }

  uint16_t raw = ((uint16_t)rx[0] << 8) | rx[1];

  // Observed behavior on some units: 0xFFFF is returned with valid CRC
  // when the temperature value is not available/supported.
  if (raw == 0xFFFF) {
    tempC = NAN;
    setError(ERR_TEMPERATURE_NOT_AVAILABLE);
    return false;
  }

  tempC = raw / 100.0f;
  clearError();
  return true;
}

bool PFLOW2001::readAmbientTemperature(float& tempC) {
  return readTemperatureCommand(CMD_READ_TEMP_AMBIENT, tempC);
}

bool PFLOW2001::readGasTemperature(float& tempC) {
  return readTemperatureCommand(CMD_READ_TEMP_GAS, tempC);
}

bool PFLOW2001::readFilteredFlow(float& emaFlowSccm,
                                 float& avgFlowSccm,
                                 int sampleCount,
                                 uint16_t sampleDelayMs,
                                 float alpha) {
  if (sampleCount <= 0 || alpha <= 0.0f || alpha > 1.0f) {
    avgFlowSccm = NAN;
    emaFlowSccm = NAN;
    setError(ERR_INVALID_ARGUMENT);
    return false;
  }

  float sum = 0.0f;
  int validCount = 0;

  for (int i = 0; i < sampleCount; i++) {
    float oneFlow = NAN;
    if (readFlow(oneFlow)) {
      if (!isnan(oneFlow)) {
        sum += oneFlow;
        validCount++;
      }
    }

    if (i < sampleCount - 1 && sampleDelayMs > 0) {
      delay(sampleDelayMs);
    }
  }

  if (validCount == 0) {
    avgFlowSccm = NAN;
    emaFlowSccm = _emaFlow;
    return false;
  }

  avgFlowSccm = sum / validCount;

  if (isnan(_emaFlow)) {
    _emaFlow = avgFlowSccm;
  } else {
    _emaFlow = alpha * avgFlowSccm + (1.0f - alpha) * _emaFlow;
  }

  emaFlowSccm = _emaFlow;
  clearError();
  return true;
}

bool PFLOW2001::readSerial(String& serialOut) {
  uint8_t rx[18];

  if (!readBytes(CMD_READ_SERIAL, rx, sizeof(rx))) {
    return false;
  }

  char ascii[13];
  int outIdx = 0;

  for (int group = 0; group < 6; group++) {
    int i = group * 3;
    uint8_t pair[2] = { rx[i], rx[i + 1] };
    uint8_t crc = rx[i + 2];

    if (crc8(pair, 2) != crc) {
      setError(ERR_CRC_SERIAL_BLOCK);
      return false;
    }

    ascii[outIdx++] = (char)rx[i];
    ascii[outIdx++] = (char)rx[i + 1];
  }

  ascii[outIdx] = '\0';
  serialOut = String(ascii);
  clearError();
  return true;
}

bool PFLOW2001::resetFlowOffset(uint16_t dummyValue) {
  return writeDataWord(CMD_RESET_OFFSET, dummyValue);
}