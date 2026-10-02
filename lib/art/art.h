#pragma once
#include <stdint.h>

namespace Art {

  // Deterministic tile-pattern cover art (size x size) drawn into the display
  // framebuffer at (x, y). The same seed strings always give the same picture.
  void drawGenerated(int16_t x, int16_t y, int16_t size, const char* seedA, const char* seedB);

  // Ordered-dither an RGB pixel to the panel's four colours and draw it.
  // Works pixel by pixel, so it fits a JPEG decoder callback (for later).
  void putDithered(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b);
  void putDitheredRgb565(int16_t x, int16_t y, uint16_t rgb565);

}