#include "Arduino.h"
#include "rs232.h"
#include <RemoteDebug.h>

// PROJECTOR SPECIFICS
const int SERIAL_BAUD_RATE = 9600;

RS232Util::RS232Util(RemoteDebug &Debug) : Debug(Debug)
{
  Serial1.begin(SERIAL_BAUD_RATE);
}

boolean RS232Util::set(uint8_t cmd1, uint8_t cmd2, uint8_t value)
{
  // Clear serial buffer
  while (Serial.available() > 0)
    Serial.read();
  // Write = 0x06, 0x14, 0x00, LSB = 0x04, MSB = 0x00, 0x34, CMD2, CMD3, Data, Checksum
  uint8_t set[10] = {0x06, 0x14, 0x00, 0x04, 0x00, 0x34, cmd1, cmd2, value, 0x00};
  checksum(set, 10);
  debugCommand((char *)"Set Command", set, 10);
  Serial1.write(set, 10);
  uint8_t response[6];
  Serial.readBytes(response, 6);
  debugCommand((char *)"Return", response, 6);
  return response[5] == 0x14; // Checksum of ACK
}

uint8_t RS232Util::get(uint8_t cmd1, uint8_t cmd2)
{
  // Clear serial buffer
  while (Serial.available() > 0)
    Serial.read();
  // Read = 0x07, 0x14, 0x00, LSB = 0x05, MSB = 0x00, 0x34, 0x00, 0x00, CMD2, CMD3, Checksum
  uint8_t get[11] = {0x07, 0x14, 0x00, 0x05, 0x00, 0x34, 0x00, 0x00, cmd1, cmd2, 0x00};
  checksum(get, 11);
  debugCommand((char *)"Get Command", get, 11);
  Serial1.write(get, 11);
  uint8_t response[9];
  Serial.readBytes(response, 9);
  debugCommand((char *)"Return", response, 9);
  return response[7]; // Single value byte
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

float RS232Util::getTemperature()
{
  return get(0x15, 0x03);
}

void RS232Util::debugCommand(char *label, uint8_t *payload, size_t size)
{
#ifndef DEBUG_DISABLED
  if (Debug.isActive(Debug.VERBOSE))
  {
    Debug.printf("%s [ ", label);
    for (size_t i = 0; i < size; i++)
    {
      Debug.printf("%X ", payload[i]);
    }
    Debug.println("]");
  }
#endif
}

void RS232Util::checksum(uint8_t *payload, size_t size)
{
  uint8_t checksum = 0x00;
  for (size_t i = 1; i < size - 1; i++)
    checksum += payload[i];
  payload[size - 1] = checksum;
}