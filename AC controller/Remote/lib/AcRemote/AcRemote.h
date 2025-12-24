#ifndef AC_REMOTE_H
#define AC_REMOTE_H

#include <Arduino.h>
#include <Preferences.h>
#include <IRremoteESP32.h>
#include <IRsend.h>
#include <IRrecv.h>

class AcRemote {
public:
  AcRemote(uint8_t txPin = 19, uint8_t rxPin = 4);
  
  bool startLearning(const String& cmd);
  void stopLearning();
  bool isLearning() { return _learning; }
  
  void setPower(bool on);
  void setTemperature(uint8_t temp);
  
  bool getPower() { return _powerOn; }
  uint8_t getTemperature() { return _temp; }
  
  bool load();

private:
  void transmitCommand(const String& cmd);
  static void irRecvCallback();
  
  uint8_t _txPin, _rxPin;
  bool _powerOn = false;
  uint8_t _temp = 24;
  bool _learning = false;
  String _learnCmd;
  
  IRsend* _irSend;
  IRrecv* _irRecv;
  Preferences _prefs;
};

extern AcRemote acRemote;

#endif