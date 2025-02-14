#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>


const char* ssid = "esp";
const char* password = "12345678";

#define PIN_D1 5
#define PIN_D2 4

// Интервал цикла (1 секунда)
unsigned long previousMillis = 0;
const long interval = 1000; 


int cycleCounter = 0;
int value_1 = 0;
int value_2 = 0;

void setup() {
  Serial.begin(115200);
  

  WiFi.begin(ssid, password);
  Serial.print("Подключение к WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nПодключено к WiFi");
  
  pinMode(PIN_D1, INPUT);
  pinMode(PIN_D2, INPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    

    int analogValue = analogRead(A0);
    int d1Value = digitalRead(PIN_D1);
    int d2Value = digitalRead(PIN_D2);
    
    if (analogValue > 500) {
      value_1 += 10;
    } else if (analogValue >= 100 && analogValue <= 500) {
      value_1 += 5;
    }
    
    if (d1Value == HIGH && d2Value == HIGH) {
      value_2 += 3;
    } else if (d1Value == HIGH || d2Value == HIGH) {
      value_2 += 2;
    } else {
      value_2 += 1;
    }
    
    cycleCounter++;
    
    Serial.print("A0: ");
    Serial.print(analogValue);
    Serial.print(" | D1: ");
    Serial.print(d1Value);
    Serial.print(" | D2: ");
    Serial.print(d2Value);
    Serial.print(" | Value_1: ");
    Serial.print(value_1);
    Serial.print(" | Value_2: ");
    Serial.println(value_2);
    

    if (cycleCounter >= 5) {
      sendPostRequest("1", value_1);
      sendPostRequest("2", value_2);
      
      cycleCounter = 0;
      value_1 = 0;
      value_2 = 0;
    }
  }
}


void sendPostRequest(String type, int value) {

  WiFiClientSecure client;
  client.setInsecure();  
  
  HTTPClient https;
  String url = "https://energy-monitor-fastapi-production.up.railway.app/create_energy/";
  
  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");
    
    String jsonBody = "{\"type\":\"" + type + "\",\"value\":\"" + String(value) + "\"}";
    Serial.println("Отправка POST запроса: " + jsonBody);
    
    int httpCode = https.POST(jsonBody);
    
    if (httpCode > 0) {
      String payload = https.getString();
      Serial.println("Ответ сервера: " + payload);
    } else {
      Serial.println("Ошибка POST запроса: " + https.errorToString(httpCode));
    }
    
    https.end();
  } else {
    Serial.println("Не удалось подключиться к " + url);
  }
}