#include <ESP8266WiFi.h>
#include <RemoteDebug.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "secrets.h"
#include "mqtt.h"
#include "rs232.h"

using namespace std;

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
const char *TOPIC_CMND_TEMPERATURE = "cmnd/projector/TEMPERATURE";

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
  mqtt.setCallback(bind(&MQTTClient::callback, this, placeholders::_1, placeholders::_2, placeholders::_3));
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
      mqtt.subscribe(TOPIC_CMND_TEMPERATURE);
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
  payload[length] = '\0';
  char *data = (char *)payload;
  debugD("Topic [%s] Payload(%d) [%s]", topic, length, data);

  if (strcmp(topic, TOPIC_CMND_POWER) == 0)
  {
    RS232.setPower(atoi(data) > 0);
    publishState();
  }
  if (strcmp(topic, TOPIC_CMND_VOLUME) == 0)
  {
    RS232.setVolume(atoi(data));
    publishState();
  }
  if (strcmp(topic, TOPIC_CMND_SOURCE) == 0)
  {
    switch (atoi(data))
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
  if (strcmp(topic, TOPIC_CMND_TEMPERATURE) == 0)
  {
    RS232.getTemperature();
  }
  if (strcmp(topic, TOPIC_CMND_COMMAND) == 0)
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error)
    {
      debugE("Deserializing JSON failed");
      debugD("Error: %s", error.c_str());
      return;
    }
    JsonArray array = doc.as<JsonArray>();
    RS232.set(array[0], array[1], array[2]);
    publishState();
  }
}

// STATE

void MQTTClient::publishState()
{
  boolean power = RS232.getPower();
  delay(1);
  Source source = RS232.getSource();
  delay(1);
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

String MQTTClient::publishCommand(char *command)
{
  return "";
}
