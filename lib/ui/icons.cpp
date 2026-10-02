#include "icons.h"

#include <Adafruit_GFX.h>
#include <math.h>

#include "display.h"

namespace {

using C = Display::Color;
inline uint16_t px(C c) { return (uint16_t)c; }

void thickLine(Adafruit_GFX& g, int16_t x0, int16_t y0, int16_t x1, int16_t y1, C c) {
  g.drawLine(x0, y0, x1, y1, px(c));
  g.drawLine(x0 + 1, y0, x1 + 1, y1, px(c));
}

void sun(Adafruit_GFX& g, int16_t cx, int16_t cy, int16_t r) {
  g.fillCircle(cx, cy, r, px(C::Yellow));
  g.drawCircle(cx, cy, r, px(C::Black));
  for (int i = 0; i < 8; i++) {
    const float a = i * 0.7853982f;
    thickLine(g,
              cx + (int16_t)lroundf(cosf(a) * r * 1.4f), cy + (int16_t)lroundf(sinf(a) * r * 1.4f),
              cx + (int16_t)lroundf(cosf(a) * r * 1.9f), cy + (int16_t)lroundf(sinf(a) * r * 1.9f),
              C::Black);
  }
}

void moon(Adafruit_GFX& g, int16_t cx, int16_t cy, int16_t r) {
  g.fillCircle(cx, cy, r, px(C::Black));
  g.fillCircle(cx + r / 2, cy - r / 3, r * 8 / 10, px(C::White));
}

void cloud(Adafruit_GFX& g, int16_t x, int16_t y, int16_t s, C c) {
  g.fillCircle(x + s * 32 / 100, y + s * 58 / 100, s * 20 / 100, px(c));
  g.fillCircle(x + s * 52 / 100, y + s * 45 / 100, s * 25 / 100, px(c));
  g.fillCircle(x + s * 76 / 100, y + s * 60 / 100, s * 17 / 100, px(c));
  g.fillRect(x + s * 32 / 100, y + s * 58 / 100, s * 44 / 100, s * 19 / 100, px(c));
}

}  // namespace

namespace Icons {

void drawWeather(int16_t x, int16_t y, int16_t s, Weather::Kind kind, bool isDay) {
  Adafruit_GFX& g = Display::canvas();

  switch (kind) {
    case Weather::Kind::Clear:
      if (isDay) sun(g, x + s / 2, y + s / 2, s * 20 / 100);
      else       moon(g, x + s / 2, y + s / 2, s * 28 / 100);
      break;

    case Weather::Kind::PartlyCloudy:
      if (isDay) sun(g, x + s * 34 / 100, y + s * 34 / 100, s * 15 / 100);
      else       moon(g, x + s * 34 / 100, y + s * 34 / 100, s * 20 / 100);
      cloud(g, x + s / 10, y + s / 5, s * 9 / 10, C::Black);
      break;

    case Weather::Kind::Cloudy:
      cloud(g, x, y, s, C::Black);
      break;

    case Weather::Kind::Fog:
      for (int i = 0; i < 4; i++) {
        g.fillRect(x + s * (10 + (i & 1) * 8) / 100, y + s * (25 + i * 16) / 100,
                   s * 70 / 100, s * 7 / 100, px(C::Black));
      }
      break;

    case Weather::Kind::Rain:
      cloud(g, x, y - s / 10, s, C::Black);
      for (int i = 0; i < 4; i++) {
        const int16_t x0 = x + s * (28 + i * 15) / 100;
        thickLine(g, x0, y + s * 72 / 100, x0 - s / 12, y + s * 92 / 100, C::Red);
      }
      break;

    case Weather::Kind::Snow:
      cloud(g, x, y - s / 10, s, C::Black);
      for (int i = 0; i < 3; i++) {                   // small plus signs as flakes
        const int16_t fx = x + s * (30 + i * 20) / 100, fy = y + s * 85 / 100, r = s / 18;
        g.drawFastHLine(fx - r, fy, 2 * r + 1, px(C::Red));
        g.drawFastVLine(fx, fy - r, 2 * r + 1, px(C::Red));
      }
      break;

    case Weather::Kind::Storm:
      cloud(g, x, y - s / 10, s, C::Black);
      g.fillTriangle(x + s * 52 / 100, y + s * 66 / 100, x + s * 38 / 100, y + s * 88 / 100,
                     x + s * 54 / 100, y + s * 86 / 100, px(C::Yellow));
      g.fillTriangle(x + s * 50 / 100, y + s * 84 / 100, x + s * 66 / 100, y + s * 74 / 100,
                     x + s * 46 / 100, y + s * 98 / 100, px(C::Yellow));
      break;

    default:
      cloud(g, x, y, s, C::Black);                    // unknown: plain cloud
      break;
  }
}

}  // namespace Icons