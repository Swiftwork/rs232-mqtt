#include <Regexp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "secrets.h"
#include "mqtt.h"
#include "rs232.h"

// PROJECTOR SPECIFICS
const String SERIAL_COMMAND_PREPEND = "\r*";
const String SERIAL_COMMAND_APPEND = "#\r";
const int SERIAL_BAUD_RATE = 9600;

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

MQTTClient::MQTTClient(WiFiClient &wifiClient)
    : wifiClient(wifiClient),
      mqtt(PubSubClient(wifiClient))
{
}

void MQTTClient::start()
{
  Serial.println("Initializing MQTT Client");
  Serial1.begin(SERIAL_BAUD_RATE);

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
  Serial.printf("MQTT server mqtt://%s:xxx@%s:%d\n", MQTT_USERNAME, MQTT_SERVER, MQTT_SERVERPORT);

  // Loop until we're reconnected
  while (!mqtt.connected())
  {
    Serial.print("Attempting MQTT connection... ");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_KEY))
    {
      Serial.println("connected");

      // Subscriptions
      mqtt.subscribe(TOPIC_CMND_POWER);
      mqtt.subscribe(TOPIC_CMND_VOLUME);
      mqtt.subscribe(TOPIC_CMND_SOURCE);
      mqtt.subscribe(TOPIC_CMND_COMMAND);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void MQTTClient::callback(char *topic, uint8_t *payload, unsigned int length)
{
  Serial.print("Topic [");
  Serial.print(topic);
  Serial.print("] received ");
  Serial.println(String((char *)payload));

  if (topic == TOPIC_CMND_POWER)
  {
    RS232.setPower(payload);
    publishState();
  }
  if (topic == TOPIC_CMND_VOLUME)
  {
    RS232.setVolume(atoi((char *)payload));
    publishState();
  }
  if (topic == TOPIC_CMND_SOURCE)
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
  if (topic == TOPIC_CMND_COMMAND)
  {
    publishCommand((char *)payload);
    publishState();
  }
}

// STATE

String MQTTClient::collectState()
{
  String current_status;
  current_status += "{";
  current_status += "\"Power\":\"" + (String)RS232.getPower() + "\"";
  current_status += ",";
  current_status += "\"Source\":\"" + (String)RS232.getSource() + "\"";
  current_status += ",";
  current_status += "\"Volume\":\"" + (String)RS232.getVolume() + "\"";
  current_status += "}";
  return current_status;
}

void MQTTClient::publishState()
{
  String state = collectState();
  mqtt.publish(TOPIC_STAT_STATE, state.c_str());
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

// HELPERS

void MQTTClient::setVolume(int target_volume)
{
  int delta = target_volume - getVolumeState();
  for (int i = 1; i <= abs(delta); i++)
  {
    if (delta > 0)
    {
      publishCommand("vol=+");
    }
    else
    {
      publishCommand("vol=-");
    }
  }
}

String MQTTClient::regex(char match_array[], String match_string)
{
  MatchState parse_result;
  char match_result[100];

  parse_result.Target(match_array);
  if (parse_result.Match(match_string.c_str()) == REGEXP_MATCHED)
  {
    parse_result.GetCapture(match_result, 0);
    return String(match_result);
  }
  else
  {
    return "UNKNOWN";
  }
}