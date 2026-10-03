#include "glance.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "art.h"
#include "bluetooth.h"
#include "cat_art.h"
#include "display.h"
#include "icons.h"
#include "network.h"
#include "ora_config.h"
#include "secrets.h"
#include "sync.h"
#include "weather.h"
#include "widgets.h"

using Display::Color;

namespace {

// ---- layout, measured from the 240x416 mockup
constexpr int16_t kRadius = 4;
constexpr int16_t kBarX = 7,  kBarY = 7,   kBarW = 226, kBarH = 22;
constexpr int16_t kRowY = 34, kRowH = 79;
constexpr int16_t kTimeX = 7,   kTimeW = 90;
constexpr int16_t kWxX   = 101, kWxW   = 132;
constexpr int16_t kMusicX = 7, kMusicY = 118, kMusicW = 226, kMusicH = 63;
constexpr int16_t kCatX = 37, kCatY = 227;

const GFXfont* const kBold = &FreeSansBold9pt7b;
const GFXfont* const kBody = &FreeSans9pt7b;
const GFXfont* const kBig  = &FreeSansBold12pt7b;

uint32_t mix(uint32_t h, uint32_t v) {
  h ^= v;
  return h * 16777619u;
}

uint32_t mixStr(uint32_t h, const char* s) {
  for (; *s; s++) h = mix(h, (uint8_t)*s);
  return mix(h, 0xFF);
}

void prepare(Adafruit_GFX& g, const GFXfont* f, Color c) {
  g.setTextSize(1);
  g.setFont(f);
  g.setTextColor((uint16_t)c);
}

void putCentered(const char* text, int16_t cx, int16_t baseline, const GFXfont* f, Color c) {
  Adafruit_GFX& g = Display::canvas();
  prepare(g, f, c);
  int16_t x1, y1;
  uint16_t w, h;
  g.getTextBounds(text, 0, baseline, &x1, &y1, &w, &h);
  g.setCursor(cx - (int16_t)w / 2 - x1, baseline);
  g.print(text);
  g.setFont(nullptr);
}

void upperInPlace(char* s) {
  for (; *s; s++) *s = (char)toupper((unsigned char)*s);
}

void formatClock(char* out, size_t n, const tm& t) {
  const int minute = (t.tm_min / GlanceCfg::CLOCK_STEP_MIN) * GlanceCfg::CLOCK_STEP_MIN;
  if (GlanceCfg::CLOCK_24H) {
    snprintf(out, n, "%02d:%02d", t.tm_hour, minute);
  } else {
    const int h12 = (t.tm_hour % 12 == 0) ? 12 : t.tm_hour % 12;
    snprintf(out, n, "%d:%02d", h12, minute);
  }
}

void card(int16_t x, int16_t y, int16_t w, int16_t h) {
  Display::canvas().drawRoundRect(x, y, w, h, kRadius, (uint16_t)Color::Black);
}

void statusPill(const char* rightText) {
  Adafruit_GFX& g = Display::canvas();
  g.fillRoundRect(kBarX, kBarY, kBarW, kBarH, kRadius, (uint16_t)Color::Red);
  prepare(g, kBold, Color::White);
  g.setCursor(kBarX + 7, kBarY + 17);
  g.print("ORA");
  g.setFont(nullptr);
  Widgets::drawRight(rightText, kBarX + kBarW - 7, kBarY + 17, kBody, Color::White);
}

void timeCard(bool haveTime, const tm& lt) {
  card(kTimeX, kRowY, kTimeW, kRowH);
  const int16_t cx = kTimeX + kTimeW / 2;
  char buf[24];

  if (!haveTime) {
    putCentered("--:--", cx, kRowY + 26, kBig, Color::Black);
    putCentered("NO", cx, kRowY + 49, kBold, Color::Black);
    putCentered("CLOCK", cx, kRowY + 70, kBold, Color::Black);
    return;
  }

  formatClock(buf, sizeof buf, lt);
  putCentered(buf, cx, kRowY + 26, kBig, Color::Black);

  char day[12];
  strftime(day, sizeof day, "%a", &lt);
  upperInPlace(day);
  if (!GlanceCfg::CLOCK_24H) {
    snprintf(buf, sizeof buf, "%s %s", day, lt.tm_hour < 12 ? "AM" : "PM");
  } else {
    snprintf(buf, sizeof buf, "%s", day);
  }
  putCentered(buf, cx, kRowY + 49, kBold, Color::Black);

  char mon[8];
  strftime(mon, sizeof mon, "%b", &lt);
  upperInPlace(mon);
  snprintf(buf, sizeof buf, "%d %s", lt.tm_mday, mon);
  putCentered(buf, cx, kRowY + 70, kBold, Color::Black);
}

void weatherCard(const Weather::Data& w) {
  card(kWxX, kRowY, kWxW, kRowH);
  Adafruit_GFX& g = Display::canvas();
  const int16_t left = kWxX + 7;
  const int16_t maxW = kWxW - 14;

  if (!w.valid) {
    Widgets::drawFitted("No weather", left, kRowY + 34, maxW, kBold, kBody, Color::Black);
    Widgets::drawFitted("Check hotspot", left, kRowY + 56, maxW, kBody, nullptr, Color::Red);
    return;
  }

  Icons::drawWeather(left - 1, kRowY + 4, 44, Weather::kind(w.code), w.isDay);

  char buf[24];
  snprintf(buf, sizeof buf, "%d", (int)lroundf(w.tempC));
  prepare(g, kBig, Color::Black);
  g.setCursor(left + 48, kRowY + 32);
  g.print(buf);
  const int16_t cx = g.getCursorX();
  g.drawCircle(cx + 5, kRowY + 17, 3, (uint16_t)Color::Black);   // degree sign (no glyph in GFX fonts)
  g.setCursor(cx + 11, kRowY + 32);
  g.print("C");
  g.setFont(nullptr);

  Widgets::drawFitted(Weather::describe(w.code), left, kRowY + 54, maxW, kBold, kBody, Color::Black);
  if (WEATHER_PLACE[0]) {
    Widgets::drawFitted(WEATHER_PLACE, left, kRowY + 72, maxW, kBold, kBody, Color::Black);
  }
}

void musicCard(const Bluetooth::TrackInfo& t) {
  card(kMusicX, kMusicY, kMusicW, kMusicH);
  const bool hasTrack = t.connected && t.title[0] != '\0';

  const char* seedA = hasTrack ? (t.album[0] ? t.album : t.title) : Bt::DEVICE_NAME;
  const char* seedB = hasTrack ? t.artist : "";
  Art::drawGenerated(kMusicX + 4, kMusicY + 4, 56, seedA, seedB);

  const int16_t tx   = kMusicX + 4 + 56 + 8;
  const int16_t maxW = kMusicX + kMusicW - 8 - tx;

  if (hasTrack) {
    Widgets::drawFitted(t.title, tx, kMusicY + 22, maxW, kBold, kBody, Color::Black);
    Widgets::drawFitted(t.artist[0] ? t.artist : "Unknown artist",
                        tx, kMusicY + 41, maxW, kBody, nullptr, Color::Red);
    // Static decoration: the screen refreshes too rarely for a live progress bar.
    Adafruit_GFX& g = Display::canvas();
    const int16_t py = kMusicY + 54;
    g.fillTriangle(tx, py - 5, tx, py + 5, tx + 9, py, (uint16_t)Color::Black);
    g.drawFastHLine(tx + 17, py, maxW - 17, (uint16_t)Color::Black);
  } else {
    Widgets::drawFitted("Nothing playing", tx, kMusicY + 22, maxW, kBold, kBody, Color::Black);
    Widgets::drawFitted(t.connected ? "Ready to play" : "Not connected",
                        tx, kMusicY + 41, maxW, kBody, nullptr, Color::Red);
  }
}

}  // namespace

uint32_t GlanceScreen::revision() const {
  uint32_t key = 0;
  if (Network::timeValid()) {
    const time_t now = time(nullptr);
    tm lt;
    localtime_r(&now, &lt);
    key = ((lt.tm_year * 366u + lt.tm_yday) * 1440u + lt.tm_hour * 60u + lt.tm_min)
              / GlanceCfg::CLOCK_STEP_MIN + 1;
  }

  const Weather::Data w = Sync::weather();
  const Bluetooth::TrackInfo t = Bluetooth::snapshot();

  uint32_t h = mix(2166136261u, key);
  h = mix(h, w.valid);
  if (w.valid) {
    h = mix(h, (uint32_t)(int32_t)lroundf(w.tempC));
    h = mix(h, (uint32_t)w.code);
    h = mix(h, w.isDay);
  }
  h = mix(h, t.connected);
  h = mixStr(h, t.title);
  h = mixStr(h, t.artist);
  h = mixStr(h, t.album);

  if (h != hash_) {
    hash_ = h;
    contentRev_++;
  }
  return contentRev_;
}

void GlanceScreen::render() {
  Display::clear(Color::White);

  const Bluetooth::TrackInfo t = Bluetooth::snapshot();
  const Weather::Data w = Sync::weather();

  const bool haveTime = Network::timeValid();
  tm lt{};
  if (haveTime) {
    const time_t now = time(nullptr);
    localtime_r(&now, &lt);
  }

  statusPill(t.connected ? "BLUETOOTH" : "WAITING");
  timeCard(haveTime, lt);
  weatherCard(w);
  musicCard(t);

  Display::drawBitmap2bpp(kCatX, kCatY, CatArt::WIDTH, CatArt::HEIGHT, CAT_ART_DATA);
}