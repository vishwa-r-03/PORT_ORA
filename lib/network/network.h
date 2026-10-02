#pragma once
#include <Arduino.h>

namespace Network {

  bool connect(uint32_t timeoutMs);               // join the hotspot from secrets.h
  void disconnect();                              // WiFi fully off
  bool syncClock(uint32_t timeoutMs);             // NTP; needs a connection
  bool timeValid();                               // has the clock ever been set?
  bool httpsGet(const char* url, String& body, uint32_t timeoutMs);

}