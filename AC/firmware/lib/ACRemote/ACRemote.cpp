#include "ACRemote.h"
#include <string.h>
#include <freertos/ringbuf.h>
#include <driver/rmt.h>

static ACRemote* _instance = nullptr;

ACRemote::ACRemote(uint8_t txPin, uint8_t rxPin) {
  _txPin = txPin;
  _rxPin = rxPin;
  _instance = this;
}

void ACRemote::_handleRxData() {
  size_t len = 0;
  rmt_item32_t* items = (rmt_item32_t*)xRingbufferReceive(_rb, &len, 0);
  if (items && len > 0) {
    size_t item_count = len / sizeof(rmt_item32_t);
    if (item_count > 1 && item_count <= MAX_RAW_LEN) {
      for (size_t i = 0; i < item_count; i++) {
        _learnedRaw[i] = items[i].duration0;
      }
      _learnedLen = item_count;
      _learning = false;
      Serial.printf("[ACRemote] Captured %d timings\n", item_count);
    }
    vRingbufferReturnItem(_rb, items);
  }
}

void ACRemote::begin() {
  // RMT RX
  rmt_config_t rx_cfg = RMT_DEFAULT_CONFIG_RX((gpio_num_t)_rxPin, _rxChannel);
  rx_cfg.clk_div = 80;
  rx_cfg.mem_block_num = 8;
  rx_cfg.rx_config.filter_en = true;
  rx_cfg.rx_config.filter_ticks_thresh = 100;
  rx_cfg.rx_config.idle_threshold = 300;
  rmt_config(&rx_cfg);
  rmt_driver_install(_rxChannel, MAX_RAW_LEN * sizeof(rmt_item32_t), 0);
  rmt_get_ringbuf_handle(_rxChannel, &_rb);
  rmt_rx_start(_rxChannel, true);

  // RMT TX
  rmt_config_t tx_cfg = RMT_DEFAULT_CONFIG_TX((gpio_num_t)_txPin, _txChannel);
  tx_cfg.clk_div = 80;
  tx_cfg.mem_block_num = 8;
  rmt_config(&tx_cfg);
  rmt_driver_install(_txChannel, 0, 0);

  // Load from flash
  Preferences prefs;
  prefs.begin("acremote", false);

  #define LOAD_RAW(key, ptr, len) \
    do { \
      size_t s = prefs.getBytesLength(key); \
      if (s > 0) { \
        ptr = new uint16_t[s / sizeof(uint16_t)]; \
        prefs.getBytes(key, ptr, s); \
        len = s / sizeof(uint16_t); \
      } \
    } while(0)

  LOAD_RAW("power_on", _powerOnRaw, _powerOnLen);
  LOAD_RAW("power_off", _powerOffRaw, _powerOffLen);
  for (int t = 16; t <= 30; t++) {
    char k[16];
    snprintf(k, sizeof(k), "temp_%d", t);
    LOAD_RAW(k, _tempRaw[t - 16], _tempLen[t - 16]);
  }
  prefs.end();

  Serial.println("[ACRemote] Ready");
}

void ACRemote::startLearning() {
  if (!_rb) return;
  _learning = true;
  _learnedLen = 0;
  size_t len = 0;
  rmt_item32_t* items = nullptr;
  while ((items = (rmt_item32_t*)xRingbufferReceive(_rb, &len, 0))) {
    vRingbufferReturnItem(_rb, items);
  }
}

void ACRemote::checkForIRSignal() {
  if (_learning && _rb) {
    _handleRxData();
  }
}

void ACRemote::savePower(bool isOn) {
  if (!_learning || _learnedLen == 0) return;
  uint16_t** target = isOn ? &_powerOnRaw : &_powerOffRaw;
  size_t* len = isOn ? &_powerOnLen : &_powerOffLen;
  if (*target) delete[] *target;
  *target = new uint16_t[_learnedLen];
  memcpy(*target, _learnedRaw, _learnedLen * sizeof(uint16_t));
  *len = _learnedLen;
  Preferences prefs;
  prefs.begin("acremote", false);
  prefs.putBytes(isOn ? "power_on" : "power_off", *target, *len * sizeof(uint16_t));
  prefs.end();
  _learning = false;
}

void ACRemote::saveTemperature(uint8_t temp) {
  if (temp < 16 || temp > 30 || !_learning || _learnedLen == 0) return;
  int idx = temp - 16;
  if (_tempRaw[idx]) delete[] _tempRaw[idx];
  _tempRaw[idx] = new uint16_t[_learnedLen];
  memcpy(_tempRaw[idx], _learnedRaw, _learnedLen * sizeof(uint16_t));
  _tempLen[idx] = _learnedLen;
  char key[16];
  snprintf(key, sizeof(key), "temp_%d", temp);
  Preferences prefs;
  prefs.begin("acremote", false);
  prefs.putBytes(key, _tempRaw[idx], _tempLen[idx] * sizeof(uint16_t));
  prefs.end();
  _acTemp = temp;
  _learning = false;
}

void ACRemote::power(bool on) {
  uint16_t* data = on ? _powerOnRaw : _powerOffRaw;
  size_t len = on ? _powerOnLen : _powerOffLen;
  if (data && len > 0) {
    rmt_item32_t* items = new rmt_item32_t[len + 1];
    for (size_t i = 0; i < len; i++) {
      items[i].level0 = 1;
      items[i].duration0 = data[i];
      items[i].level1 = 0;
      items[i].duration1 = 560;
    }
    items[len].val = 0;
    rmt_write_items(_txChannel, items, len + 1, true);
    delete[] items;
    _acPower = on;
  }
}

void ACRemote::setTemperature(uint8_t temp) {
  if (temp < 16 || temp > 30) return;
  int idx = temp - 16;
  if (_tempRaw[idx] && _tempLen[idx] > 0) {
    rmt_item32_t* items = new rmt_item32_t[_tempLen[idx] + 1];
    for (size_t i = 0; i < _tempLen[idx]; i++) {
      items[i].level0 = 1;
      items[i].duration0 = _tempRaw[idx][i];
      items[i].level1 = 0;
      items[i].duration1 = 560;
    }
    items[_tempLen[idx]].val = 0;
    rmt_write_items(_txChannel, items, _tempLen[idx] + 1, true);
    delete[] items;
    _acTemp = temp;
  }
}

bool ACRemote::hasPowerOn() const { return _powerOnRaw != nullptr; }
bool ACRemote::hasPowerOff() const { return _powerOffRaw != nullptr; }
bool ACRemote::hasTemperature(uint8_t temp) const {
  if (temp < 16 || temp > 30) return false;
  return _tempRaw[temp - 16] != nullptr;
}

void ACRemote::getStatus(bool* power, uint8_t* temp) {
  if (power) *power = _acPower;
  if (temp) *temp = _acTemp;
}