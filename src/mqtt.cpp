#include <Regexp.h>
#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#include "../secrets.h"
#include "mqtt.h"

// PROJECTOR SPECIFICS
const String SERIAL_COMMAND_PREPEND = "\r*";
const String SERIAL_COMMAND_APPEND = "#\r";
const int SERIAL_BAUD_RATE = 9600;

MQTTClient::MQTTClient(WiFiClient *wifiClient)
    : wifiClient(wifiClient),
      mqtt(Adafruit_MQTT_Client(wifiClient, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY)),
      state_pub(Adafruit_MQTT_Publish(&mqtt, "stat/projector/STATE")),
      command_pub(Adafruit_MQTT_Publish(&mqtt, "stat/projector/COMMAND")),
      power_sub(Adafruit_MQTT_Subscribe(&mqtt, "cmnd/projector/POWER")),
      volume_sub(Adafruit_MQTT_Subscribe(&mqtt, "cmnd/projector/VOLUME")),
      source_sub(Adafruit_MQTT_Subscribe(&mqtt, "cmnd/projector/SOURCE")),
      command_sub(Adafruit_MQTT_Subscribe(&mqtt, "cmnd/projector/COMMAND"))
{
}

void MQTTClient::start()
{
  Serial.println("Initializing MQTT Client");
  Serial1.begin(SERIAL_BAUD_RATE);

  // Setup MQTT subscriptions
  mqtt.subscribe(&power_sub);
  mqtt.subscribe(&volume_sub);
  mqtt.subscribe(&source_sub);
  mqtt.subscribe(&command_sub);
}

void MQTTClient::connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }

  Serial.print((String) "Connecting to MQTT server mqtt://" + MQTT_USERNAME + ":xxx@" + MQTT_SERVER + ":" + MQTT_SERVERPORT + "... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.print("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000); // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }
  Serial.println("MQTT connected!");
}

void MQTTClient::update()
{
  MQTTClient::connect();
  Adafruit_MQTT_Subscribe *subscription;

  while ((subscription = mqtt.readSubscription(5000)))
  {
    if (subscription == &power_sub)
    {
      publishCommand("pow=" + String((char *)power_sub.lastread));
      publishState();
    }
    if (subscription == &volume_sub)
    {
      setVolume(atoi((char *)volume_sub.lastread));
      publishState();
    }
    if (subscription == &source_sub)
    {
      publishCommand("sour=" + String((char *)source_sub.lastread));
      publishState();
    }
    if (subscription == &command_sub)
    {
      publishCommand((char *)command_sub.lastread);
      publishState();
    }
  }
  publishState();
}

// STATE

String MQTTClient::getPowerState()
{
  char current_power_status[50];
  serialCommand("pow=?").toCharArray(current_power_status, 50);
  String result = regex(current_power_status, "POW=([^#]*)");
  if (result == "UNKNOWN")
  {
    return "OFF";
  }
  else
  {
    return result;
  }
}

String MQTTClient::getSourceState()
{
  char current_source_status[50];
  serialCommand("sour=?").toCharArray(current_source_status, 50);
  return regex(current_source_status, "SOUR=([^#]*)");
}

int MQTTClient::getVolumeState()
{
  char current_volume_status[50];
  serialCommand("vol=?").toCharArray(current_volume_status, 50);
  return (regex(current_volume_status, "VOL=([^#]*)")).toInt();
}

String MQTTClient::collectState()
{
  String current_status;
  current_status += "{";
  current_status += "\"POWER\":\"" + getPowerState() + "\"";
  current_status += ",";
  current_status += "\"SOURCE\":\"" + getSourceState() + "\"";
  current_status += ",";
  current_status += "\"VOLUME\":\"" + String(getVolumeState()) + "\"";
  current_status += "}";
  return current_status;
}

void MQTTClient::publishState()
{
  state_pub.publish(collectState().c_str());
}

// COMMANDS

String MQTTClient::serialCommand(String serial_command)
{
  Serial1.print(SERIAL_COMMAND_PREPEND);
  Serial1.print(serial_command);
  Serial1.print(SERIAL_COMMAND_APPEND);

  String serial_response = Serial.readString();
  return serial_response;
}

String MQTTClient::publishCommand(String command)
{
  String response = serialCommand(command);
  String status_response = "{\"COMMAND\":\"" + command + "\",\"RESPONSE\":\"" + response + "\"}";
  command_pub.publish(status_response.c_str());
  return response;
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