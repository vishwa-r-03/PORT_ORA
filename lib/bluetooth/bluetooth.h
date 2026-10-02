#pragma once
#include <stdint.h>

namespace Bluetooth {

  struct TrackInfo {
    char     title[64];
    char     artist[48];
    char     album[48];
    uint32_t durationMs;
    bool     connected;
  };

  void      begin();        // starts the A2DP sink
  TrackInfo snapshot();     // thread-safe copy of the current state
  uint32_t  revision();     // increments whenever something visible changes

}