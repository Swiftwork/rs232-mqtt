#ifndef PX747_H
#define PX747_H

#include <map>

class PX747Device
{
public:
  PX747Device();

private:
  static std::map<char *, std::array<uint8_t, 3>> commands;
};

#endif