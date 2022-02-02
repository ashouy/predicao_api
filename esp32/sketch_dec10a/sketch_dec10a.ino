#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

#define DHTPIN 26
#define DHTTYPE DHT11
#define TYPE "ESP32"

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "brisa-2200735";
const char* password = "qbyoag8g";
const char* serverName = "https://us-east-1.aws.data.mongodb-api.com/app/api-predicao-xunrm/endpoint/predicao/save?secret=87194584rm";


StaticJsonDocument<500> doc;



void setup() {
  Serial.begin(115200);

  Serial.print("Conectando a : ");
  Serial.print(ssid);
  Serial.print(" com a senha ");
  Serial.println(password);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado ao WiFi Com o endereço de IP: ");
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
      Serial.println("não foi possível ler do sensor");
      return;
    }
    gerarObjetoASerEnviado(h, t);
    enviarObjeto();
    delay(60000);
  } else {
    ESP.restart();
  }
}

void gerarObjetoASerEnviado(float humidade, float temperatura) {
  doc["id_node"] = "ESP";
  doc["temperatura"] = temperatura;
  doc["umidade"] = humidade;
}

void enviarObjeto() {
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
