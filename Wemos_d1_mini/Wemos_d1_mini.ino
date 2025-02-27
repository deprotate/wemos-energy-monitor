#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <math.h> 

const char* ssid = "esp";
const char* password = "12345678";

const double solar_coefficient = 0.035;
const unsigned int max_consumed_value = 20;
const unsigned int mid_consumed_value = 10;
const unsigned int min_consumed_value = 5;

#define PIN_D1 5
#define PIN_D2 4

// Интервал цикла (1 секунда)
unsigned long previous_millis = 0;
const long interval = 1000; 


int cycle_counter = 0;
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
  unsigned long current_millis = millis();
  if (current_millis - previous_millis >= interval) {
    previous_millis = current_millis;
    

    int analog_value = analogRead(A0);
    int d1_value = digitalRead(PIN_D1);
    int d2_value = digitalRead(PIN_D2);
    
    value_2 = round(analog_value * solar_coefficient);
    
    if (d1_value == HIGH && d2_value == HIGH) {
      value_1 += max_consumed_value;
    } else if (d1_value == HIGH || d2_value == HIGH) {
      value_1 += mid_consumed_value;
    } else {
      value_1 += min_consumed_value;
    }
    
    cycle_counter++;
    
    Serial.print("A0: ");
    Serial.print(analog_value);
    Serial.print(" | D1: ");
    Serial.print(d1_value);
    Serial.print(" | D2: ");
    Serial.print(d2_value);
    Serial.print(" | Value_1: ");
    Serial.print(value_1);
    Serial.print(" | Value_2: ");
    Serial.println(value_2);
    

    if (cycle_counter >= 5) {
      sendPostRequest("1", value_1);
      sendPostRequest("2", value_2);
      
      cycle_counter = 0;
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
