#ifndef MQTT_H
#define MQTT_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class MQTTClient
{
public:
  MQTTClient(WiFiClient &wifiClient);
  void start();
  void update();

private:
  WiFiClient &wifiClient;

  // MQTT
  PubSubClient mqtt;
  void connect();
  void callback(char *topic, uint8_t *payload, unsigned int length);

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