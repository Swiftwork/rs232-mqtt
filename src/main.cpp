#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <RemoteDebug.h>

#include "secrets.h"
#include "mqtt.h"
#include "ota.h"

// WIFI
WiFiClient wifiClient;
RemoteDebug Debug;
OTAClient otaClient(Debug);
MQTTClient mqttClient(wifiClient, Debug);

// LED
unsigned long ledPrevMs = 0;
const long ledIntervalMs = 1000;
int ledState = LOW;

void wifiConnect()
{
  Serial.begin(115200);

  // Connect to WiFi access point.
  debugI("Connecting to %s", WIFI_SSID);
  WiFi.hostname(WIFI_HOSTNAME);
  wifi_station_set_hostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    rdebugI(".");
  }
  rdebugIln(" connected");
  debugI("IP address: %s", WiFi.localIP().toString().c_str());
}

void setup()
{
  wifiConnect();

  Debug.begin(WIFI_HOSTNAME);
  Debug.setResetCmdEnabled(true);

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

  Debug.handle();
}