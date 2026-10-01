#pragma once
#include <stdint.h>

class Adafruit_GFX;

namespace Display {

  // Values are the panel's native 2-bit codes.
  enum class Color : uint8_t {
    Black  = 0b00,
    White  = 0b01,
    Yellow = 0b10,
    Red    = 0b11
  };

  bool begin();                                   // one-time hardware setup + blank framebuffer
  void clear(Color c = Color::White);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Color c);
  void drawText(int16_t x, int16_t y, const char* text,
                Color c = Color::Black, uint8_t size = 1);
  void drawBitmap2bpp(int16_t x, int16_t y, int16_t w, int16_t h,
                      const uint8_t* data);       // packed 4 px/byte, row stride = ceil(w/4)
  void loadFrame(const uint8_t* data);            // full-screen 2bpp image (ignores rotation)
  Adafruit_GFX& canvas();                         // lines, circles, GFX fonts, rotation, ...

  bool canRefresh();                              // 180 s guideline elapsed?
  bool needsMaintenanceRefresh();                 // >24 h since last refresh?
  bool refresh(bool force = false);               // blocks ~12-20 s, sleeps panel afterwards
  void sleep();
}