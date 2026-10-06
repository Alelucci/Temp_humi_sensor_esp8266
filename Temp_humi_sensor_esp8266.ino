#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <LiquidCrystal.h>

#include "secrets.h"

#define DHTPIN D1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(D2, D3, D4, D5, D6, D7);

#define NEBU_PIN D0

int cicli = 299;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  dht.begin();

  pinMode(NEBU_PIN, OUTPUT);
  digitalWrite(NEBU_PIN, HIGH);

  lcd.begin(16, 2);
  
  printAll("Connecting to");
  lcd.setCursor(0, 1);
  printAll(ssid);
  
  WiFi.begin(ssid, password);
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    printAll("WiFi connected!");
    lcd.setCursor(0, 1);
    printAll(String(WiFi.localIP().toString()).c_str());
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    printAll("WiFi failed");
  }
  
  delay(5000);
  lcd.clear();
}

void loop() {
  lcd.clear();
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  do{
    delay(2000);
    temp = dht.readTemperature();
    humidity = dht.readHumidity();

    if (!isnan(temp) && !isnan(humidity)) {
      Serial.print("Temp: ");
      Serial.print(temp);
      Serial.print("°C  Humidity: ");
      Serial.println(humidity);
    
      lcd.setCursor(0, 0);
      lcd.print("Temp:    ");
      lcd.print(temp);
      lcd.print(" C");
    
      lcd.setCursor(0, 1);
      lcd.print("Humid:   ");
      lcd.print(humidity);
      lcd.print(" %");
   
    } else {
      lcd.setCursor(0, 0);
      printAll("DHT Error");
    }
  
    cicli++;

  }while(cicli<300);

  sendToThingSpeak(temp, humidity);
  cicli = 0;

  if (humidity < humi_min) {
    digitalWrite(NEBU_PIN, LOW);
  } else if (humidity > humi_max) {
    digitalWrite(NEBU_PIN, HIGH);
  }

}

void sendToThingSpeak(float temp, float humidity) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    return;
  }
  
  WiFiClient client;
  HTTPClient http;
  
  String url = BASE_URL;
  url += "&field1=";
  url += temp;
  url += "&field2=";
  url += humidity;
  
  Serial.print("Sending to ThingSpeak...");
  http.begin(client, url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    Serial.println(" OK!");
  } else {
    Serial.print(" FAILED (");
    Serial.print(httpCode);
    Serial.println(")");
  }
  http.end();
}

void printAll(const char* message){
  lcd.print(message);
  Serial.println(message);
}