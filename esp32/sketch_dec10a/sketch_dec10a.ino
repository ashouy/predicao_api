#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define TYPE "ESP32"

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
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado ao WiFi Com o endereço de IP: ");
  Serial.println(WiFi.localIP());

}

void loop() {

  gerarObjetoASerEnviado();
  enviarObjeto();

  delay(60000);
}

void gerarObjetoASerEnviado(){
  doc["id_node"] = "ESP";
  doc["sensors"]["0"]["type"] = "temperature";
  doc["sensors"]["0"]["value"] = random(18,30);
  doc["sensors"]["1"]["type"] = "humidity";
  doc["sensors"]["1"]["value"] = random(50,90);
}

void enviarObjeto(){
      if(WiFi.status()== WL_CONNECTED){
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
