#ifndef MQTT_H
#define MQTT_H

#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

class MQTTClient
{
public:
  MQTTClient(WiFiClient *wifiClient);
  void start();
  void update();

private:
  WiFiClient *wifiClient;

  // MQTT
  Adafruit_MQTT_Client mqtt;
  Adafruit_MQTT_Publish state_pub;
  Adafruit_MQTT_Publish command_pub;
  Adafruit_MQTT_Subscribe power_sub;
  Adafruit_MQTT_Subscribe volume_sub;
  Adafruit_MQTT_Subscribe source_sub;
  Adafruit_MQTT_Subscribe command_sub;

  // INIT
  void connect();

  // STATE
  String getPowerState();
  String getSourceState();
  int getVolumeState();
  String collectState();
  void publishState();

  // COMMANDS
  String serialCommand(String serial_command);
  String publishCommand(String command);
  void setVolume(int target_volume);

  // HELPER
  String regex(char match_array[], String match_string);
};

#endif