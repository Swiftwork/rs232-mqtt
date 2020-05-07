#include <ESP8266WiFi.h>

#include "secrets.h"
#include "src/mqtt.h"
#include "src/ota.h"

// WIFI
WiFiClient wifiClient;
OTAClient otaClient;
MQTTClient mqttClient(&wifiClient);

void setup()
{
  wifiConnect();
  otaClient.start();
  mqttClient.start();
}

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
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  otaClient.update();
  mqttClient.update();
}