#ifndef RS232_H
#define RS232_H

enum Source
{
  DSub = 0x00,
  HDMI1 = 0x03,
  HDMI2 = 0x07,
};

class RS232Util
{
public:
  RS232Util();
  static boolean set(uint8_t cmd1, uint8_t cmd2, uint8_t value);
  static uint8_t get(uint8_t cmd1, uint8_t cmd2);

  static boolean setPower(boolean on);
  static boolean getPower();

  static boolean setSource(Source source);
  static Source getSource();

  static boolean setVolume(float percent);
  static float getVolume();

private:
  static uint8_t checksum(uint8_t payload[]);
};

extern RS232Util RS232;

#endif