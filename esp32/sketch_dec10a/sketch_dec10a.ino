#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define TYPE "ESP32"

const char* ssid = "brisa-2200735";
const char* password = "r5boncn6";
const char* serverName = "https://us-east-1.aws.data.mongodb-api.com/app/application-0-niuwc/endpoint/predicao_api?secret=87194584rm";


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

  delay(100000);
}

void gerarObjetoASerEnviado(){
  doc["id_node"] = "ESP";
  doc["datetime"] = "09/12/2021 10:20"; 
  doc["sensors"]["0"]["type"] = "temperature";
  doc["sensors"]["0"]["value"] = 25;
  doc["sensors"]["1"]["type"] = "humidity";
  doc["sensors"]["1"]["value"] = 75;
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
