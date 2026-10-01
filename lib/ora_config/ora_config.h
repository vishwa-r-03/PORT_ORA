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
  // Native buffer layout of the panel driver
  constexpr uint16_t NATIVE_WIDTH  = 240;
  constexpr uint16_t NATIVE_HEIGHT = 416;
  // 0 = portrait, 1 or 3 = landscape (416x240), 2 = portrait upside-down
  constexpr uint8_t  ROTATION = 0;

  constexpr uint32_t MIN_REFRESH_MS = 180000UL;
  constexpr uint32_t MAINTENANCE_MS = 24UL * 60UL * 60UL * 1000UL;
}