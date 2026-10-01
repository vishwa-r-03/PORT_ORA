#pragma once
#include <Arduino.h>

namespace Pins {
  // E-paper (4-wire SPI)
  constexpr uint8_t EPD_SCK  = 18;
  constexpr uint8_t EPD_MOSI = 23;  // DIN
  constexpr uint8_t EPD_CS   = 5;
  constexpr uint8_t EPD_DC   = 21;
  constexpr uint8_t EPD_RST  = 22;
  constexpr uint8_t EPD_BUSY = 4;
  constexpr uint8_t EPD_PWR  = 33;
  // TODO: input, I2S, SD, battery sense
}

namespace DisplayCfg {
  constexpr uint16_t WIDTH  = 416;
  constexpr uint16_t HEIGHT = 240;
  constexpr uint32_t MIN_REFRESH_MS = 180000UL;                     // vendor guidance; lower while testing
  constexpr uint32_t MAINTENANCE_MS = 24UL * 60UL * 60UL * 1000UL;  // refresh at least daily
}