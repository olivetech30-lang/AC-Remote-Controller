#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "AcRemote.h"

// CONFIGURE THESE!
const char* WIFI_SSID = "YourWiFi";
const char* WIFI_PASSWORD = "YourPassword";
const char* VERCEL_URL = "https://your-project.vercel.app";

unsigned long lastPoll = 0;
const unsigned long POLL_INTERVAL = 2000;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to Wi-Fi");
}

void loop() {
  if (millis() - lastPoll > POLL_INTERVAL) {
    lastPoll = millis();
    
    HTTPClient http;
    http.begin(String(VERCEL_URL) + "/api/ac/command");
    int httpCode = http.GET();
    
    if (httpCode == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(256);
      deserializeJson(doc, payload);
      
      if (!doc["executed"].as<bool>()) {
        String cmd = doc["command"].as<String>();
        
        if (cmd == "power_on") {
          acRemote.setPower(true);
        } else if (cmd == "power_off") {
          acRemote.setPower(false);
        } else if (cmd.startsWith("temp_")) {
          uint8_t temp = cmd.substring(5).toInt();
          acRemote.setTemperature(temp);
        }
        
        // Acknowledge
        http.begin(String(VERCEL_URL) + "/api/ac/ack");
        http.addHeader("Content-Type", "application/json");
        http.POST(payload);
      }
    }
    http.end();
  }
}