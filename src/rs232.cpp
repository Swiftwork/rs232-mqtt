#include "Arduino.h"
#include "rs232.h"

RS232Util::RS232Util()
{
}

boolean RS232Util::set(uint8_t cmd1, uint8_t cmd2, uint8_t value)
{
  // Write = 0x06, 0x14, 0x00, LSB = 0x04, MSB = 0x00, 0x34, CMD2, CMD3, Data, Checksum
  uint8_t set[] = {0x06, 0x14, 0x00, 0x04, 0x00, 0x34, cmd1, cmd2, value, 0x00};
  set[sizeof(set) - 1] = checksum(set);
  Serial1.write(set, 10);
  uint8_t response[5];
  Serial1.readBytes(response, 5);
  return response[4] == 0x14;
}

uint8_t RS232Util::get(uint8_t cmd1, uint8_t cmd2)
{
  // Read = 0x07, 0x14, 0x00, LSB = 0x05, MSB = 0x00, 0x34, 0x00, 0x00, CMD2, CMD3, Checksum
  uint8_t get[] = {0x06, 0x14, 0x00, 0x05, 0x00, 0x34, 0x00, 0x00, cmd1, cmd2, 0x00};
  get[sizeof(get) - 1] = checksum(get);
  Serial1.write(get, 11);
  uint8_t response[8];
  Serial1.readBytes(response, 8);
  return response[6];
}

boolean RS232Util::setPower(boolean on)
{
  if (on)
    return set(0x11, 0x00, 0x00);
  else
    return set(0x11, 0x01, 0x00);
}

boolean RS232Util::getPower()
{
  return get(0x11, 0x00);
}

boolean RS232Util::setSource(Source source)
{
  return set(0x13, 0x01, source);
}

Source RS232Util::getSource()
{
  return (Source)get(0x13, 0x01);
}

boolean RS232Util::setVolume(float percent)
{
  return set(0x13, 0x2A, floor(0xFF * percent));
}

float RS232Util::getVolume()
{
  return get(0x14, 0x03) / 0xFF;
}

uint8_t RS232Util::checksum(uint8_t payload[])
{
  uint8_t checksum = 0x00;
  for (size_t i = 1; i < sizeof(payload) - 1; i++)
  {
    checksum += payload[i];
  }
  return checksum;
}
