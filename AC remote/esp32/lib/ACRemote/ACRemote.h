#ifndef ACREMOTE_H
#define ACREMOTE_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <LittleFS.h>

#define RAW_BUF_LEN 1024

class ACRemote {
public:
    ACRemote(uint8_t rxPin, uint8_t txPin);

    void begin();
    void handleLearning();

    void startLearning(const char* name);
    bool send(const char* name);
    
    String listCommands();
    bool isLearning() { return _learning; }

private:
    IRrecv _irrecv;
    IRsend _irsend;
    decode_results _results;

    bool _learning = false;
    String _currentCmd;

    void _saveCommand(const char* name, uint16_t* raw, uint16_t len);
    bool _loadCommand(const char* name, uint16_t* raw, uint16_t &len);
    String _getFilename(const char* name);
};

#endif