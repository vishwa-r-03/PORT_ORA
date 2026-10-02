#include "art.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <limits.h>

#include "display.h"

namespace {

using C = Display::Color;

constexpr int kGrid = 8;                       // 8x8 tiles

struct Pair { C bg; C fg; };

// Only combinations that read well (no yellow on white).
const Pair kPairs[] = {
  {C::White, C::Black}, {C::Black, C::White}, {C::White, C::Red},
  {C::Red,   C::White}, {C::Black, C::Red},   {C::Black, C::Yellow},
  {C::Red,   C::Black},
};
constexpr uint32_t kNumPairs = sizeof(kPairs) / sizeof(kPairs[0]);

struct Rng {
  uint32_t s;
  uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
};

uint32_t hashSeed(const char* a, const char* b) {      // FNV-1a over both strings
  uint32_t h = 2166136261u;
  const char* parts[2] = {a, b};
  for (const char* p : parts) {
    for (; p && *p; p++) { h ^= (uint8_t)*p; h *= 16777619u; }
    h ^= 0xFF; h *= 16777619u;                          // separator
  }
  return h ? h : 1;
}

inline uint16_t px(C c) { return (uint16_t)c; }

// Quarter disc centred on one tile corner, drawn per pixel so nothing leaves the tile.
void quarter(Adafruit_GFX& g, int16_t x, int16_t y, int16_t s, uint8_t corner, C fg) {
  const int16_t cx = (corner & 1) ? s - 1 : 0;
  const int16_t cy = (corner & 2) ? s - 1 : 0;
  const int32_t r2 = (int32_t)(s - 1) * (s - 1);
  for (int16_t j = 0; j < s; j++) {
    for (int16_t i = 0; i < s; i++) {
      const int32_t dx = i - cx, dy = j - cy;
      if (dx * dx + dy * dy <= r2) g.drawPixel(x + i, y + j, px(fg));
    }
  }
}

void drawTile(Adafruit_GFX& g, int16_t x, int16_t y, int16_t s,
              uint8_t motif, uint8_t corner, const Pair& p) {
  g.fillRect(x, y, s, s, px(p.bg));
  const int16_t m = s / 2;

  switch (motif) {
    case 0:                                             // disc
      g.fillCircle(x + m, y + m, m - 2, px(p.fg));
      break;
    case 1:                                             // quarter disc
      quarter(g, x, y, s, corner, p.fg);
      break;
    case 2: {                                           // half-square triangle
      const int16_t x0 = x, x1 = x + s - 1, y0 = y, y1 = y + s - 1;
      switch (corner & 3) {
        case 0:  g.fillTriangle(x0, y0, x1, y0, x0, y1, px(p.fg)); break;
        case 1:  g.fillTriangle(x0, y0, x1, y0, x1, y1, px(p.fg)); break;
        case 2:  g.fillTriangle(x0, y0, x0, y1, x1, y1, px(p.fg)); break;
        default: g.fillTriangle(x1, y0, x0, y1, x1, y1, px(p.fg)); break;
      }
      break;
    }
    case 3:                                             // ring
      g.fillCircle(x + m, y + m, m - 2, px(p.fg));
      g.fillCircle(x + m, y + m, m - 7, px(p.bg));
      break;
    default:                                            // bar
      if (corner & 1) g.fillRect(x, y + s / 3, s, s / 3, px(p.fg));
      else            g.fillRect(x + s / 3, y, s / 3, s, px(p.fg));
      break;
  }
}

// ---- dithering --------------------------------------------------------------
struct Rgb { uint8_t r, g, b; };

// Index == panel colour code: 0 black, 1 white, 2 yellow, 3 red.
// These are guesses; tune them by eye against what your panel really shows.
const Rgb kPal[4] = { {25, 25, 25}, {230, 230, 220}, {215, 185, 40}, {165, 40, 40} };

const uint8_t kBayer[4][4] = {
  { 0,  8,  2, 10},
  {12,  4, 14,  6},
  { 3, 11,  1,  9},
  {15,  7, 13,  5},
};

}  // namespace

namespace Art {

void drawGenerated(int16_t x, int16_t y, int16_t size, const char* seedA, const char* seedB) {
  Adafruit_GFX& g = Display::canvas();
  Rng rng{hashSeed(seedA, seedB)};
  const int16_t tile = size / kGrid;

  // Separate statements: argument evaluation order is unspecified in C++.
  const uint32_t i1 = rng.next() % kNumPairs;
  uint32_t i2 = rng.next() % kNumPairs;
  if (i2 == i1) i2 = (i2 + 1) % kNumPairs;

  for (int ty = 0; ty < kGrid; ty++) {
    for (int tx = 0; tx < kGrid; tx++) {
      const uint32_t v = rng.next();
      const Pair& pair   = (v & 1) ? kPairs[i1] : kPairs[i2];
      const uint8_t motif  = (uint8_t)((v >> 1) % 5);
      const uint8_t corner = (uint8_t)((v >> 8) & 3);
      drawTile(g, x + tx * tile, y + ty * tile, tile, motif, corner, pair);
    }
  }
}

void putDithered(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b) {
  const int t = ((int)kBayer[y & 3][x & 3] * 2 - 15) * 2;   // roughly -30..+30
  int best = 0;
  int bestD = INT_MAX;
  for (int i = 0; i < 4; i++) {
    const int dr = r + t - kPal[i].r;
    const int dg = g + t - kPal[i].g;
    const int db = b + t - kPal[i].b;
    const int d = dr * dr + dg * dg + db * db;
    if (d < bestD) { bestD = d; best = i; }
  }
  Display::canvas().drawPixel(x, y, (uint16_t)best);
}

void putDitheredRgb565(int16_t x, int16_t y, uint16_t c) {
  const uint8_t r = (uint8_t)(((c >> 11) & 0x1F) * 255 / 31);
  const uint8_t g = (uint8_t)(((c >> 5)  & 0x3F) * 255 / 63);
  const uint8_t b = (uint8_t)((c & 0x1F) * 255 / 31);
  putDithered(x, y, r, g, b);
}

}  // namespace Art