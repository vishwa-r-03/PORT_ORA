#include <Arduino.h>
#include "ora_config.h"
#include "display.h"

void setup() {
  Serial.begin(115200);
  Display::begin();
  Display::clear(Display::Color::White);
  Display::fillRect(0, 0, 416, 36, Display::Color::Red);
  Display::drawText(10, 8, "Ora", Display::Color::White, 3);
  Display::drawText(10, 60, "Hello e-ink", Display::Color::Black, 2);
  Display::refresh(true);   // slow and flickery: that is normal
}
void loop() {}