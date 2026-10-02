#pragma once
#include <stdint.h>
#include "weather.h"

namespace Sync {

  bool          runOnce();                           // blocking: WiFi -> NTP -> weather -> WiFi off
  void          startTask();                         // periodic background sync
  void          setDeferCheck(bool (*shouldDefer)()); // return true while music is playing
  Weather::Data weather();                           // thread-safe copy
  uint32_t      ageMs();                             // ms since last good sync, UINT32_MAX if never

}