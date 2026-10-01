#include "display.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <string.h>

#include "ora_config.h"
#include "panel.h"

namespace {

constexpr uint16_t kWidth       = DisplayCfg::WIDTH;    // 416
constexpr uint16_t kHeight      = DisplayCfg::HEIGHT;   // 240
constexpr size_t   kBytesPerRow = kWidth / 4;           // 4 pixels per byte
constexpr size_t   kFrameBytes  = kBytesPerRow * kHeight; // 24,960 bytes

static_assert(kWidth % 4 == 0, "Width must be a multiple of 4 (byte-aligned rows)");

uint8_t  frame[kFrameBytes];
bool     initialised   = false;
bool     everRefreshed = false;
uint32_t lastRefreshMs = 0;

// Write one pixel (physical coordinates, 2-bit colour code).
inline void setPixelRaw(int16_t x, int16_t y, uint8_t code) {
  if (x < 0 || y < 0 || x >= (int16_t)kWidth || y >= (int16_t)kHeight) return;
  const size_t  idx   = (size_t)y * kBytesPerRow + (x >> 2);
  const uint8_t shift = 6 - 2 * (x & 3);          // first pixel in the top bits
  uint8_t b = frame[idx];
  b &= (uint8_t)~(0x03u << shift);
  b |= (uint8_t)((code & 0x03u) << shift);
  frame[idx] = b;
}

// Adafruit_GFX gives us text, lines, shapes, fonts and rotation for free.
class EpdCanvas : public Adafruit_GFX {
 public:
  EpdCanvas() : Adafruit_GFX(kWidth, kHeight) {}

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    switch (getRotation()) {
      case 1: { int16_t t = x; x = kWidth - 1 - y; y = t; break; }
      case 2: { x = kWidth - 1 - x; y = kHeight - 1 - y; break; }
      case 3: { int16_t t = x; x = y; y = kHeight - 1 - t; break; }
      default: break;
    }
    setPixelRaw(x, y, (uint8_t)color);
  }

  void fillScreen(uint16_t color) override {      // fast path
    memset(frame, (color & 0x03) * 0x55, kFrameBytes); // code replicated across 4 pixels
  }
};

EpdCanvas gfx;

}  // namespace

namespace Display {

bool begin() {
  if (!Panel::begin()) return false;
  gfx.setRotation(0);                             // native landscape, 416x240
  gfx.setTextWrap(false);
  gfx.fillScreen((uint16_t)Color::White);
  initialised = true;
  return true;
}

void clear(Color c) {
  gfx.fillScreen((uint16_t)c);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Color c) {
  gfx.fillRect(x, y, w, h, (uint16_t)c);
}

void drawText(int16_t x, int16_t y, const char* text, Color c, uint8_t size) {
  gfx.setCursor(x, y);
  gfx.setTextColor((uint16_t)c);
  gfx.setTextSize(size);
  gfx.print(text);
}

void drawBitmap2bpp(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* data) {
  if (!data || w <= 0 || h <= 0) return;
  const size_t stride = ((size_t)w + 3) / 4;
  for (int16_t j = 0; j < h; j++) {
    for (int16_t i = 0; i < w; i++) {
      const uint8_t b    = data[(size_t)j * stride + (i >> 2)];
      const uint8_t code = (b >> (6 - 2 * (i & 3))) & 0x03;
      gfx.drawPixel(x + i, y + j, code);
    }
  }
}

void loadFrame(const uint8_t* data) {
  if (data) memcpy(frame, data, kFrameBytes);
}

Adafruit_GFX& canvas() {
  return gfx;
}

bool canRefresh() {
  return !everRefreshed || (millis() - lastRefreshMs) >= DisplayCfg::MIN_REFRESH_MS;
}

bool needsMaintenanceRefresh() {
  // millis() resets on reboot/deep sleep; persist the timestamp in RTC memory later.
  return everRefreshed && (millis() - lastRefreshMs) >= DisplayCfg::MAINTENANCE_MS;
}

bool refresh(bool force) {
  if (!initialised) return false;
  if (!force && !canRefresh()) return false;

  if (!Panel::wake()) return false;               // re-init (panel sleeps after every update)
  Panel::show(frame);                             // blocks until the panel releases BUSY
  Panel::sleep();                                 // never leave the panel powered

  lastRefreshMs = millis();
  everRefreshed = true;
  return true;
}

void sleep() {
  Panel::sleep();
}

}  // namespace Display