#ifndef RS232_H
#define RS232_H

#include <RemoteDebug.h>

enum Source
{
  DSub = 0x00,
  HDMI1 = 0x03,
  HDMI2 = 0x07,
};

class RS232Util
{
public:
  RS232Util(RemoteDebug &Debug);

  boolean set(uint8_t cmd1, uint8_t cmd2, uint8_t value);
  uint8_t get(uint8_t cmd1, uint8_t cmd2);

  boolean setPower(boolean on);
  boolean getPower();

  boolean setSource(Source source);
  Source getSource();

  boolean setVolume(float percent);
  float getVolume();

private:
  RemoteDebug &Debug;

  void debugCommand(char *label, uint8_t *payload, size_t size);
  void checksum(uint8_t *payload, size_t size);
};

#endif