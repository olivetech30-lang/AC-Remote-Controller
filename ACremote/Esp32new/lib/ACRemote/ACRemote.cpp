#include "ACREMOTE.h"

ACRemote::ACRemote(uint8_t rxPin, uint8_t txPin)
: _irrecv(rxPin, RAW_BUF_LEN, 50, true),
  _irsend(txPin) {}

void ACRemote::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("❌ LittleFS mount failed!");
        return;
    }

    _irrecv.enableIRIn();
    _irsend.begin();

    Serial.println("✅ ACRemote ready. LittleFS mounted.");
}

void ACRemote::startLearning(const char* name) {
    _learning = true;
    _currentCmd = name;

    Serial.println("\n==============================");
    Serial.print("📡 Learning started for: ");
    Serial.println(name);
    Serial.println("👉 Press AC remote button...");
}

void ACRemote::handleLearning() {
    if (!_learning) return;

    if (_irrecv.decode(&_results)) {
        uint16_t len = _results.rawlen - 1;
        if (len > RAW_BUF_LEN) len = RAW_BUF_LEN;

        static uint16_t raw[RAW_BUF_LEN];
        for (uint16_t i = 1; i <= len; i++) {
            raw[i - 1] = _results.rawbuf[i] * kRawTick;
        }

        _saveCommand(_currentCmd.c_str(), raw, len);

        Serial.print("✅ Learning complete for: ");
        Serial.println(_currentCmd);
        Serial.print("📊 Signal length: ");
        Serial.println(len);

        _learning = false;
        _currentCmd = "";

        _irrecv.resume();
    }
}

bool ACRemote::send(const char* name) {
    uint16_t raw[RAW_BUF_LEN];
    uint16_t len = 0;

    if (!_loadCommand(name, raw, len)) {
        Serial.print("❌ Command not found: ");
        Serial.println(name);
        return false;
    }

    _irsend.sendRaw(raw, len, 38);

    Serial.print("📤 Sent command: ");
    Serial.println(name);
    return true;
}

void ACRemote::_saveCommand(const char* name, uint16_t* raw, uint16_t len) {
    String filename = _getFilename(name);
    File f = LittleFS.open(filename, "w");
    if (!f) {
        Serial.print("❌ Failed to write: ");
        Serial.println(filename);
        return;
    }

    f.write((uint8_t*)&len, sizeof(len));
    f.write((uint8_t*)raw, len * sizeof(uint16_t));
    f.close();
    
    Serial.print("💾 Saved to: ");
    Serial.println(filename);
}

bool ACRemote::_loadCommand(const char* name, uint16_t* raw, uint16_t &len) {
    String filename = _getFilename(name);
    File f = LittleFS.open(filename, "r");
    if (!f) return false;

    f.read((uint8_t*)&len, sizeof(len));
    if (len > RAW_BUF_LEN) len = RAW_BUF_LEN;

    f.read((uint8_t*)raw, len * sizeof(uint16_t));
    f.close();
    return true;
}

String ACRemote::_getFilename(const char* name) {
    String fname = "/";
    fname += String(name);
    fname.toLowerCase();
    fname += ".raw";
    return fname;
}

String ACRemote::listCommands() {
    String result = "";
    File root = LittleFS.open("/");
    if (!root) return result;

    File file = root.openNextFile();
    bool first = true;
    
    while (file) {
        String filename = String(file.name());
        if (filename.endsWith(".raw")) {
            if (!first) result += ",";
            filename.replace("/", "");
            filename.replace(".raw", "");
            filename.toUpperCase();
            result += "\"" + filename + "\"";
            first = false;
        }
        file = root.openNextFile();
    }
    
    return result;
}