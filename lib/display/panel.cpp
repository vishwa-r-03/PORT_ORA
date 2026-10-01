#include "panel.h"
#include <Arduino.h>

namespace Panel {
  bool begin() { Serial.println("[panel] begin"); return true; }
  bool wake()  { Serial.println("[panel] wake");  return true; }
  void show(const uint8_t*) { Serial.println("[panel] show"); delay(500); }
  void sleep() { Serial.println("[panel] sleep"); }
}