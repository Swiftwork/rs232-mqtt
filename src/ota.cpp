#include <ArduinoOTA.h>

#include "ota.h"

using namespace std;

OTAClient::OTAClient()
{
}

void OTAClient::start()
{
  Serial.println("Initializing Over-The-Air Client");
  ArduinoOTA.onStart(bind(&OTAClient::onStart, this));
  ArduinoOTA.onEnd(bind(&OTAClient::onEnd, this));
  ArduinoOTA.onProgress(bind(&OTAClient::onProgress, this, placeholders::_1, placeholders::_2));
  ArduinoOTA.onError(bind(&OTAClient::onError, this, placeholders::_1));
  ArduinoOTA.begin();
}

void OTAClient::update()
{
  ArduinoOTA.handle();
}

void OTAClient::onStart()
{
  String type;
  if (ArduinoOTA.getCommand() == U_FLASH)
  {
    type = "sketch";
  }
  else
  { // U_SPIFFS
    type = "filesystem";
  }
  Serial.println("Start OTA updating " + type);
}

void OTAClient::onEnd()
{
  Serial.println("\nEnd");
}

void OTAClient::onProgress(unsigned int progress, unsigned int total)
{
  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
}

void OTAClient::onError(ota_error_t error)
{
  Serial.printf("Error[%u]: ", error);
  if (error == OTA_AUTH_ERROR)
  {
    Serial.println("Auth Failed");
  }
  else if (error == OTA_BEGIN_ERROR)
  {
    Serial.println("Begin Failed");
  }
  else if (error == OTA_CONNECT_ERROR)
  {
    Serial.println("Connect Failed");
  }
  else if (error == OTA_RECEIVE_ERROR)
  {
    Serial.println("Receive Failed");
  }
  else if (error == OTA_END_ERROR)
  {
    Serial.println("End Failed");
  }
}
