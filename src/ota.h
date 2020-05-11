#ifndef OTA_H
#define OTA_H

#include <RemoteDebug.h>
#include <ArduinoOTA.h>

class OTAClient
{
public:
  OTAClient(RemoteDebug &Debug);

  void start();
  void update();

private:
  RemoteDebug &Debug;

  void onStart();
  void onEnd();
  void onProgress(unsigned int progress, unsigned int total);
  void onError(ota_error_t error);
};

#endif