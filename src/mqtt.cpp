#include <ESP8266WiFi.h>
#include <RemoteDebug.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "secrets.h"
#include "mqtt.h"
#include "rs232.h"

// MQTT
const long STATE_INTERVAL_MS = 5000;
unsigned long statePrevMs = 0;

// TOPICS
const char *TOPIC_STAT_STATE = "stat/projector/STATE";
const char *TOPIC_STAT_COMMAND = "stat/projector/COMMAND";
const char *TOPIC_CMND_POWER = "cmnd/projector/POWER";
const char *TOPIC_CMND_VOLUME = "cmnd/projector/VOLUME";
const char *TOPIC_CMND_SOURCE = "cmnd/projector/SOURCE";
const char *TOPIC_CMND_COMMAND = "cmnd/projector/COMMAND";

MQTTClient::MQTTClient(WiFiClient &wifiClient, RemoteDebug &Debug)
    : wifiClient(wifiClient),
      Debug(Debug),
      RS232(RS232Util(Debug)),
      mqtt(PubSubClient(wifiClient))
{
}

void MQTTClient::start()
{
  debugI("Initializing MQTT Client");

  mqtt.setServer(MQTT_SERVER, MQTT_SERVERPORT);
  mqtt.setCallback([this](char *topic, uint8_t *payload, unsigned int length) {
    callback(topic, payload, length);
  });
}

void MQTTClient::update()
{
  if (!mqtt.connected())
  {
    connect();
  }
  mqtt.loop();

  // Publish current state every stateIntervalMs
  unsigned long now = millis();
  if (now - statePrevMs > STATE_INTERVAL_MS)
  {
    statePrevMs = now;
    publishState();
  }
}

void MQTTClient::connect()
{
  debugI("MQTT server mqtt://%s:xxx@%s:%d\n", MQTT_USERNAME, MQTT_SERVER, MQTT_SERVERPORT);

  // Loop until we're reconnected
  while (!mqtt.connected())
  {
    debugI("Attempting MQTT connection... ");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_KEY))
    {
      debugI("connected");

      // Subscriptions
      mqtt.subscribe(TOPIC_CMND_POWER);
      mqtt.subscribe(TOPIC_CMND_VOLUME);
      mqtt.subscribe(TOPIC_CMND_SOURCE);
      mqtt.subscribe(TOPIC_CMND_COMMAND);
    }
    else
    {
      debugW("failed, rc=%d try again in 5 seconds", mqtt.state());
      delay(5000);
    }
  }
}

void MQTTClient::callback(char *topic, uint8_t *payload, unsigned int length)
{
  debugD("Topic [%s]", topic);

  if (strcmp(topic, TOPIC_CMND_POWER) == 0)
  {
    RS232.setPower(atoi((char *)payload) > 0);
    publishState();
  }
  if (strcmp(topic, TOPIC_CMND_VOLUME) == 0)
  {
    RS232.setVolume(atoi((char *)payload));
    publishState();
  }
  if (strcmp(topic, TOPIC_CMND_SOURCE) == 0)
  {
    switch (atoi((char *)payload))
    {
    case 0:
      RS232.setSource(Source::DSub);
      break;
    case 1:
      RS232.setSource(Source::HDMI1);
      break;
    case 2:
      RS232.setSource(Source::HDMI2);
      break;
    }
    publishState();
  }
  if (strcmp(topic, TOPIC_CMND_COMMAND) == 0)
  {
    publishCommand((char *)payload);
    publishState();
  }
}

// STATE

void MQTTClient::publishState()
{
  return;
  boolean power = RS232.getPower();
  Source source = RS232.getSource();
  float volume = RS232.getVolume();
  StaticJsonDocument<256> doc;
  doc["power"] = power;
  doc["source"] = source;
  doc["volume"] = volume;
  char state[256];
  size_t n = serializeJson(doc, state, 256);
  debugD("publish: %s", state);
  mqtt.publish(TOPIC_STAT_STATE, state, n);
}

// COMMANDS

String MQTTClient::publishCommand(String command)
{
  /*
  String response = serialCommand(command);
  String statusResponse = "{\"COMMAND\":\"" + command + "\",\"RESPONSE\":\"" + response + "\"}";
  mqtt.publish(TOPIC_STAT_COMMAND, statusResponse.c_str());
  return response;
  */
  return "";
}
