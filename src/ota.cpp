#include <RemoteDebug.h>
#include <ArduinoOTA.h>

#include "ota.h"

using namespace std;

OTAClient::OTAClient(RemoteDebug &Debug) : Debug(Debug)
{
}

void OTAClient::start()
{
  debugI("Initializing Over-The-Air Client");
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
  debugI("Start OTA updating %s", type.c_str());
}

void OTAClient::onEnd()
{
  debugI("OTA update ended");
}

void OTAClient::onProgress(unsigned int progress, unsigned int total)
{
  debugD("Progress: %u%%\r", (progress / (total / 100)));
}

void OTAClient::onError(ota_error_t error)
{
  debugE("Error[%u]: ", error);
  if (error == OTA_AUTH_ERROR)
  {
    debugE("Auth Failed");
  }
  else if (error == OTA_BEGIN_ERROR)
  {
    debugE("Begin Failed");
  }
  else if (error == OTA_CONNECT_ERROR)
  {
    debugE("Connect Failed");
  }
  else if (error == OTA_RECEIVE_ERROR)
  {
    debugE("Receive Failed");
  }
  else if (error == OTA_END_ERROR)
  {
    debugE("End Failed");
  }
}
