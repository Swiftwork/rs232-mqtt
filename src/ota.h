#ifndef OTA_H
#define OTA_H

#include <ArduinoOTA.h>

class OTAClient
{
public:
  OTAClient();
  void start();
  void update();

private:
  void onStart();
  void onEnd();
  void onProgress(unsigned int progress, unsigned int total);
  void onError(ota_error_t error);
};

#endif