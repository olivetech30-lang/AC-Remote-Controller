#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ACRemote.h>

// === Wi-Fi Credentials ===
const char* ssid = "HUAWEI Baig";
const char* password = "cd6c696d";

ACRemote ac;
WebServer server(80);

// === HTTP Handlers ===

void handleStatus() {
  bool power; uint8_t temp;
  ac.getStatus(&power, &temp);
  String json = "{";
  json += "\"power\":" + String(power ? "true" : "false") + ",";
  json += "\"temp\":" + String(temp) + ",";
  json += "\"hasPowerOn\":" + String(ac.hasPowerOn() ? "true" : "false") + ",";
  json += "\"hasPowerOff\":" + String(ac.hasPowerOff() ? "true" : "false");
  for (int t = 16; t <= 30; t++) {
    json += ",\"hasTemp" + String(t) + "\":" + String(ac.hasTemperature(t) ? "true" : "false");
  }
  json += "}";
  server.send(200, "application/json", json);
}

void handlePower() {
  if (!server.hasArg("state")) {
    server.send(400, "text/plain", "Missing ?state=on|off");
    return;
  }
  String state = server.arg("state");
  if (state == "on") ac.power(true);
  else if (state == "off") ac.power(false);
  else {
    server.send(400, "text/plain", "Invalid state");
    return;
  }
  server.send(200, "text/plain", "OK");
}

void handleTemp() {
  if (!server.hasArg("temp")) {
    server.send(400, "text/plain", "Missing ?temp=24");
    return;
  }
  int temp = server.arg("temp").toInt();
  if (temp < 16 || temp > 30) {
    server.send(400, "text/plain", "Temp must be 16–30");
    return;
  }
  ac.setTemperature(temp);
  server.send(200, "text/plain", "OK");
}

void handleLearnPowerOn() {
  ac.startLearning();
  server.send(200, "text/plain", "Learning Power ON");
}

void handleLearnPowerOff() {
  ac.startLearning();
  server.send(200, "text/plain", "Learning Power OFF");
}

void handleSavePowerOn() {
  if (ac.isLearning()) {
    server.send(400, "text/plain", "Still learning");
    return;
  }
  ac.savePower(true);
  server.send(200, "text/plain", "Saved Power ON");
}

void handleSavePowerOff() {
  if (ac.isLearning()) {
    server.send(400, "text/plain", "Still learning");
    return;
  }
  ac.savePower(false);
  server.send(200, "text/plain", "Saved Power OFF");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// === Setup & Loop ===

void setup() {
  Serial.begin(115200);
  delay(1000);
  ac.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  server.on("/status", handleStatus);
  server.on("/power", handlePower);
  server.on("/temp", handleTemp);
  server.on("/learn/poweron", handleLearnPowerOn);
  server.on("/learn/poweroff", handleLearnPowerOff);
  server.on("/save/poweron", handleSavePowerOn);
  server.on("/save/poweroff", handleSavePowerOff);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  ac.checkForIRSignal(); // Poll for IR learning
  server.handleClient();
}