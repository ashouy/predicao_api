#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

#define DHTPIN 26
#define DHTTYPE DHT11
#define TYPE "ESP32"

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "network-id";
const char* password = "network-password";
const char* serverName = "https://server-name";


StaticJsonDocument<500> doc;

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to: ");
  Serial.print(ssid);
  Serial.print(" with password: ");
  Serial.println(password);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi with IP Address: ");
  Serial.println(WiFi.localIP());
  dht.begin();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float h;
    float t;
    
    t = dht.readTemperature();
    h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("Can't read from sensor");
      return;
    }
    createObjectToSend(h, t);
    sendObject();
    delay(60000);
  } else {
    ESP.restart();
  }
}

void createObjectToSend(float humidity, float temperature) {
  doc["id_node"] = "ESP";
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
}

void sendObject() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String json;
    serializeJson(doc, json);

    Serial.println(json);
    int httpResponseCode = http.POST(json);
    Serial.println(httpResponseCode);
  }
}
