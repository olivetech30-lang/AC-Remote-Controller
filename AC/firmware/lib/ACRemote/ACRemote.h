#ifndef ACREMOTE_H
#define ACREMOTE_H

#include <Arduino.h>
#include <Preferences.h>
#include <driver/rmt.h>
#include <freertos/ringbuf.h>

#define MAX_RAW_LEN 512

class ACRemote {
public:
  ACRemote(uint8_t txPin = 4, uint8_t rxPin = 10);
  void begin();
  void startLearning();
  void savePower(bool isOn);
  void saveTemperature(uint8_t temp);
  void power(bool on);
  void setTemperature(uint8_t temp);
  bool isLearning() const { return _learning; }
  bool hasPowerOn() const;
  bool hasPowerOff() const;
  bool hasTemperature(uint8_t temp) const;
  void getStatus(bool* power, uint8_t* temp);
  void checkForIRSignal();

private:
  void _handleRxData();
  uint8_t _txPin, _rxPin;
  rmt_channel_t _rxChannel = RMT_CHANNEL_0;
  rmt_channel_t _txChannel = RMT_CHANNEL_1;
  RingbufHandle_t _rb = NULL;

  bool _learning = false;
  uint16_t _learnedRaw[MAX_RAW_LEN];
  size_t _learnedLen = 0;

  uint16_t* _powerOnRaw = nullptr;
  size_t _powerOnLen = 0;
  uint16_t* _powerOffRaw = nullptr;
  size_t _powerOffLen = 0;
  uint16_t* _tempRaw[15] = {};
  size_t _tempLen[15] = {};

  bool _acPower = false;
  uint8_t _acTemp = 24;
};

#endif