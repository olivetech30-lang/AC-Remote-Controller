#include "AcRemote.h"

AcRemote acRemote;

AcRemote::AcRemote(uint8_t txPin, uint8_t rxPin) 
  : _txPin(txPin), _rxPin(rxPin) {
  _irSend = new IRsend(_txPin);
  _irRecv = new IRrecv(_rxPin);
  _irSend->begin();
  load();
}

bool AcRemote::startLearning(const String& cmd) {
  if (cmd != "power_on" && cmd != "power_off" && 
      !cmd.startsWith("temp_")) return false;
      
  _learnCmd = cmd;
  _learning = true;
  _irRecv->enableIRIn();
  return true;
}

void AcRemote::irRecvCallback() {
  if (!acRemote._learning) return;
  
  decode_results results;
  if (acRemote._irRecv->decode(&results)) {
    acRemote._prefs.begin("ac_remote", false);
    acRemote._prefs.putBytes(
      (acRemote._learnCmd + "_data").c_str(), 
      results.rawbuf, 
      results.rawlen * sizeof(uint32_t)
    );
    acRemote._prefs.putUInt(
      (acRemote._learnCmd + "_len").c_str(), 
      results.rawlen
    );
    acRemote._prefs.end();
    
    acRemote._learning = false;
    acRemote._irRecv->disableIRIn();
    Serial.println("✅ Learned: " + acRemote._learnCmd);
  }
}

void AcRemote::setPower(bool on) {
  transmitCommand(on ? "power_on" : "power_off");
  _powerOn = on;
}

void AcRemote::setTemperature(uint8_t temp) {
  if (temp < 16 || temp > 30) return;
  transmitCommand("temp_" + String(temp));
  _temp = temp;
}

void AcRemote::transmitCommand(const String& cmd) {
  _prefs.begin("ac_remote", true);
  uint16_t len = _prefs.getUInt((cmd + "_len").c_str(), 0);
  if (len == 0) {
    Serial.println("⚠️ Command not learned: " + cmd);
    _prefs.end();
    return;
  }
  
  uint32_t* buf = new uint32_t[len];
  _prefs.getBytes((cmd + "_data").c_str(), buf, len * sizeof(uint32_t));
  _prefs.end();
  
  uint16_t* timings = new uint16_t[len];
  for (int i = 0; i < len; i++) {
    timings[i] = (uint16_t)buf[i];
  }
  
  _irSend->sendRaw(timings, len, 38);
  
  delete[] buf;
  delete[] timings;
}

bool AcRemote::load() {
  return true;
}