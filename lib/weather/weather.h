#pragma once
#include <time.h>

namespace Weather {

  enum class Kind { Clear, PartlyCloudy, Cloudy, Fog, Rain, Snow, Storm, Unknown };

  struct Data {
    bool   valid = false;
    float  tempC = 0, highC = 0, lowC = 0;
    int    code  = -1;                 // WMO weather code
    bool   isDay = true;
    time_t fetchedAt = 0;
  };

  bool        fetch(Data& out);        // WiFi must be connected
  Kind        kind(int wmoCode);
  const char* describe(int wmoCode);

}