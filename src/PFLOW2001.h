#ifndef PFLOW2001_H
#define PFLOW2001_H

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

class PFLOW2001 {
public:
  static constexpr uint8_t DEFAULT_ADDRESS_7BIT = 0x50;
  static constexpr uint8_t ALT_ADDRESS_7BIT = 0x28;

  enum ErrorCode : uint8_t {
    OK = 0,
    ERR_I2C_WRITE_COMMAND,
    ERR_I2C_REQUEST,
    ERR_I2C_READ_LENGTH,
    ERR_CRC_WORD1,
    ERR_CRC_WORD2,
    ERR_CRC_SERIAL_BLOCK,
    ERR_CRC_TEMPERATURE,
    ERR_TEMPERATURE_NOT_AVAILABLE,
    ERR_NO_ACK,
    ERR_INVALID_ARGUMENT
  };

  explicit PFLOW2001(TwoWire& wirePort = Wire);

  bool begin(uint8_t address = DEFAULT_ADDRESS_7BIT, uint32_t clockHz = 100000);
  bool ping();

  bool readFlow(float& flowSccm);
  bool readSerial(String& serialOut);
  bool resetFlowOffset(uint16_t dummyValue = 0xAA55);

  bool readAmbientTemperature(float& tempC);
  bool readGasTemperature(float& tempC);

  bool readFilteredFlow(float& emaFlowSccm,
                        float& avgFlowSccm,
                        int sampleCount = 20,
                        uint16_t sampleDelayMs = 10,
                        float alpha = 0.1f);

  void resetFilter();

  uint8_t getAddress() const;

  ErrorCode lastError() const;
  const char* lastErrorString() const;
  void clearError();

  static uint8_t crc8(const uint8_t* data, size_t len);
  static bool scanKnownAddresses(TwoWire& wirePort, uint8_t& foundAddress);

private:
  TwoWire* _wire;
  uint8_t _address;
  ErrorCode _lastError;
  float _emaFlow;

  static constexpr uint16_t CMD_READ_SERIAL       = 0x0030;
  static constexpr uint16_t CMD_READ_FLOW         = 0x003A;
  static constexpr uint16_t CMD_READ_TEMP_AMBIENT = 0x003F;
  static constexpr uint16_t CMD_READ_TEMP_GAS     = 0x0040;
  static constexpr uint16_t CMD_RESET_OFFSET      = 0x00F0;

  bool writeCommandNoStop(uint16_t cmd);
  bool readBytes(uint16_t cmd, uint8_t* buf, size_t len);
  bool writeDataWord(uint16_t cmd, uint16_t value);
  bool readTemperatureCommand(uint16_t cmd, float& tempC);
  void setError(ErrorCode err);
};

#endif