#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "secrets.h"
#include "mqtt.h"
#include "ota.h"

// WIFI
WiFiClient wifiClient;
OTAClient otaClient;
MQTTClient mqttClient(wifiClient);

// LED
unsigned long ledPrevMs = 0;
const long ledIntervalMs = 1000;
int ledState = LOW;

void wifiConnect()
{
  Serial.begin(115200);

  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  WiFi.hostname("px747-controller");
  wifi_station_set_hostname("px747-controller");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  wifiConnect();
  otaClient.start();
  mqttClient.start();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  otaClient.update();
  mqttClient.update();

  // Blink without delay
  unsigned long currentMs = millis();
  if (currentMs - ledPrevMs >= ledIntervalMs)
  {
    ledPrevMs = currentMs;
    ledState = not(ledState);
    digitalWrite(LED_BUILTIN, ledState);
  }
}