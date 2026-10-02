#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------- Pins
namespace Pins {
  // E-paper (bit-banged by the vendor driver)
  constexpr uint8_t EPD_SCK  = 18;
  constexpr uint8_t EPD_MOSI = 23;   // DIN
  constexpr uint8_t EPD_CS   = 5;
  constexpr uint8_t EPD_DC   = 21;
  constexpr uint8_t EPD_RST  = 22;
  constexpr uint8_t EPD_BUSY = 4;
  constexpr uint8_t EPD_PWR  = 33;
  // TODO: input, I2S, SD, battery sense
}

// ---------------------------------------------------------------- Display
namespace DisplayCfg {
  constexpr uint16_t NATIVE_WIDTH  = 240;    // panel buffer layout (portrait)
  constexpr uint16_t NATIVE_HEIGHT = 416;
  constexpr uint8_t  ROTATION = 0;           // 0 = portrait 240x416

  // Fast init: shorter refresh. Compare colour quality and ghosting against normal.
  constexpr bool     FAST_REFRESH = true;

  // true  = refresh allowed every 30 s (bench testing only)
  // false = vendor guidance, 180 s minimum (use this for daily carrying)
  constexpr bool     BENCH_MODE = true;
  constexpr uint32_t MIN_REFRESH_MS = BENCH_MODE ? 30000UL : 180000UL;
  constexpr uint32_t MAINTENANCE_MS = 24UL * 60UL * 60UL * 1000UL;
}

// ---------------------------------------------------------------- Layout (portrait)
namespace Layout {
  constexpr int16_t W = 240, H = 416;
  constexpr int16_t STATUS_H = 24;
  constexpr int16_t ART_X = 0, ART_Y = 28, ART_SIZE = 240;
  constexpr int16_t META_X = 12, TEXT_W = 216;
  constexpr int16_t TITLE_BASELINE  = 300;
  constexpr int16_t ARTIST_BASELINE = 328;
  constexpr int16_t ALBUM_BASELINE  = 352;
  constexpr int16_t FOOTER_RULE_Y   = 366;
  constexpr int16_t FOOTER_BASELINE = 398;
}

// ---------------------------------------------------------------- Bluetooth
namespace Bt {
  constexpr char DEVICE_NAME[] = "Ora-Player";
  constexpr bool LOG_METADATA  = true;       // print metadata to Serial
}

// ---------------------------------------------------------------- UI
namespace UiCfg {
  constexpr uint32_t DEBOUNCE_MS = 8000;     // wait this long after the last visible change
}