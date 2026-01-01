#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "ACREMOTE.h"

// WiFi credentials - EDIT THESE!
const char* ssid = "FFC-MISC";           // ← Change to your WiFi name
const char* password = "";   // ← Change to your WiFi password

#define IR_RX 10
#define IR_TX 4

ACRemote ac(IR_RX, IR_TX);
WebServer server(80);

// CORS headers for cross-origin requests
void setCORSHeaders() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// Handle preflight requests
void handleOptions() {
    setCORSHeaders();
    server.send(200);
}

// Learn IR command
void handleLearn() {
    setCORSHeaders();
    
    if (!server.hasArg("cmd")) {
        server.send(400, "application/json", "{\"error\":\"Missing cmd parameter\"}");
        return;
    }
    
    String cmd = server.arg("cmd");
    ac.startLearning(cmd.c_str());
    
    server.send(200, "application/json", "{\"status\":\"learning\",\"command\":\"" + cmd + "\"}");
}

// Send IR command
void handleSend() {
    setCORSHeaders();
    
    if (!server.hasArg("cmd")) {
        server.send(400, "application/json", "{\"error\":\"Missing cmd parameter\"}");
        return;
    }
    
    String cmd = server.arg("cmd");
    bool success = ac.send(cmd.c_str());
    
    if (success) {
        server.send(200, "application/json", "{\"status\":\"sent\",\"command\":\"" + cmd + "\"}");
    } else {
        server.send(404, "application/json", "{\"error\":\"Command not found\",\"command\":\"" + cmd + "\"}");
    }
}

// Power control endpoint
void handlePower() {
    setCORSHeaders();
    
    String state = server.hasArg("state") ? server.arg("state") : "toggle";
    String cmd = (state == "on") ? "POWER_ON" : "POWER_OFF";
    
    bool success = ac.send(cmd.c_str());
    
    if (success) {
        server.send(200, "application/json", "{\"status\":\"success\",\"state\":\"" + state + "\"}");
    } else {
        server.send(404, "application/json", "{\"error\":\"Command not learned yet\"}");
    }
}

// Temperature control endpoint
void handleTemp() {
    setCORSHeaders();
    
    if (!server.hasArg("temp")) {
        server.send(400, "application/json", "{\"error\":\"Missing temp parameter\"}");
        return;
    }
    
    String temp = server.arg("temp");
    String cmd = "TEMP_" + temp;
    
    bool success = ac.send(cmd.c_str());
    
    if (success) {
        server.send(200, "application/json", "{\"status\":\"success\",\"temperature\":" + temp + "}");
    } else {
        server.send(404, "application/json", "{\"error\":\"Temperature command not learned yet\"}");
    }
}

// List all learned commands
void handleList() {
    setCORSHeaders();
    
    String commands = ac.listCommands();
    server.send(200, "application/json", "{\"commands\":[" + commands + "]}");
}

// Status endpoint
void handleStatus() {
    setCORSHeaders();
    
    String json = "{\"status\":\"online\",\"ip\":\"" + WiFi.localIP().toString() + "\",\"learning\":" + (ac.isLearning() ? "true" : "false") + "}";
    server.send(200, "application/json", json);
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    // Initialize AC Remote
    ac.begin();

    // Connect to WiFi
    Serial.println("\n==============================");
    Serial.println("Connecting to WiFi...");
    Serial.print("SSID: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi Connected!");
        Serial.print("📍 IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.println("==============================");
    } else {
        Serial.println("\n❌ WiFi Connection Failed!");
        Serial.println("Check your SSID and password");
        Serial.println("==============================");
    }

    // Setup HTTP endpoints
    server.on("/learn", HTTP_GET, handleLearn);
    server.on("/send", HTTP_GET, handleSend);
    server.on("/power", HTTP_GET, handlePower);
    server.on("/temp", HTTP_GET, handleTemp);
    server.on("/list", HTTP_GET, handleList);
    server.on("/status", HTTP_GET, handleStatus);
    
    // Handle OPTIONS for CORS
    server.onNotFound([]() {
        if (server.method() == HTTP_OPTIONS) {
            handleOptions();
        } else {
            setCORSHeaders();
            server.send(404, "application/json", "{\"error\":\"Not found\"}");
        }
    });

    server.begin();
    Serial.println("\n🌐 HTTP server started");
    Serial.println("\n📡 ESP32 AC Remote Ready!");
    Serial.println("\nAvailable Endpoints:");
    Serial.println("  GET /status");
    Serial.println("  GET /learn?cmd=POWER_ON");
    Serial.println("  GET /send?cmd=POWER_ON");
    Serial.println("  GET /power?state=on");
    Serial.println("  GET /temp?temp=24");
    Serial.println("  GET /list");
    Serial.println("\n==============================");
}

void loop() {
    server.handleClient();
    ac.handleLearning();
}