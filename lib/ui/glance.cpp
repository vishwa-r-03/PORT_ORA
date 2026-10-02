#include "glance.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

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

uint32_t mix(uint32_t h, uint32_t v) {
  h ^= v;
  return h * 16777619u;
}

void put(const char* text, int16_t x, int16_t baseline, const GFXfont* f, Color c) {
  Adafruit_GFX& g = Display::canvas();
  g.setTextSize(1);
  g.setFont(f);
  g.setTextColor((uint16_t)c);
  g.setCursor(x, baseline);
  g.print(text);
  g.setFont(nullptr);
}

void formatClock(char* out, size_t n, const tm& t) {
  const int step = GlanceCfg::CLOCK_STEP_MIN;
  const int minute = (t.tm_min / step) * step;
  if (GlanceCfg::CLOCK_24H) {
    snprintf(out, n, "%02d:%02d", t.tm_hour, minute);
  } else {
    const int h12 = (t.tm_hour % 12 == 0) ? 12 : t.tm_hour % 12;
    snprintf(out, n, "%d:%02d %s", h12, minute, t.tm_hour < 12 ? "AM" : "PM");
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
  uint32_t h = mix(2166136261u, key);
  h = mix(h, w.valid);
  if (w.valid) {
    h = mix(h, (uint32_t)(int32_t)lroundf(w.tempC));
    h = mix(h, (uint32_t)(int32_t)lroundf(w.highC));
    h = mix(h, (uint32_t)(int32_t)lroundf(w.lowC));
    h = mix(h, (uint32_t)w.code);
    h = mix(h, w.isDay);
  }

  if (h != hash_) {
    hash_ = h;
    contentRev_++;
  }
  return contentRev_;
}

void GlanceScreen::render() {
  Adafruit_GFX& g = Display::canvas();
  Display::clear(Color::White);
  Widgets::drawStatusBar("Glance");

  // ---- time and date
  const bool haveTime = Network::timeValid();
  tm lt{};
  if (haveTime) {
    const time_t now = time(nullptr);
    localtime_r(&now, &lt);
  }

  char buf[40];
  if (haveTime) {
    formatClock(buf, sizeof buf, lt);
    put(buf, Layout::META_X, 84, &FreeSansBold24pt7b, Color::Black);
    strftime(buf, sizeof buf, "%a, %d %b", &lt);
    put(buf, Layout::META_X, 116, &FreeSansBold12pt7b, Color::Red);
  } else {
    put("--:--", Layout::META_X, 84, &FreeSansBold24pt7b, Color::Black);
    put("Waiting for network", Layout::META_X, 116, &FreeSans9pt7b, Color::Red);
  }
  Widgets::drawDivider(132);

  // ---- weather
  const Weather::Data w = Sync::weather();
  if (w.valid) {
    Icons::drawWeather(Layout::META_X, 146, 80, Weather::kind(w.code), w.isDay);

    snprintf(buf, sizeof buf, "%d", (int)lroundf(w.tempC));
    g.setTextSize(1);
    g.setFont(&FreeSansBold18pt7b);
    g.setTextColor((uint16_t)Color::Black);
    g.setCursor(108, 198);
    g.print(buf);
    const int16_t cx = g.getCursorX();
    g.drawCircle(cx + 6, 198 - 24, 4, (uint16_t)Color::Black);   // degree sign (no glyph in GFX fonts)
    g.setCursor(cx + 13, 198);
    g.print("C");
    g.setFont(nullptr);

    put(Weather::describe(w.code), 108, 222, &FreeSans9pt7b, Color::Black);
    snprintf(buf, sizeof buf, "H %d   L %d", (int)lroundf(w.highC), (int)lroundf(w.lowC));
    put(buf, 108, 244, &FreeSansBold9pt7b, Color::Red);

    if (WEATHER_PLACE[0]) {
      Widgets::drawFitted(WEATHER_PLACE, Layout::META_X, 276, Layout::TEXT_W,
                          Widgets::bodyFont(), nullptr, Color::Black);
    }
  } else {
    put("Weather unavailable", Layout::META_X, 190, &FreeSans9pt7b, Color::Black);
    put("Check hotspot and secrets.h", Layout::META_X, 214, &FreeSans9pt7b, Color::Red);
  }

  // ---- agenda (placeholder until a source is chosen)
  Widgets::drawDivider(296);
  put("Today", Layout::META_X, 322, &FreeSansBold9pt7b, Color::Red);
  put("Nothing planned", Layout::META_X, 348, &FreeSans9pt7b, Color::Black);

  // ---- footer
  if (w.valid && w.fetchedAt > 0) {
    tm ft;
    localtime_r(&w.fetchedAt, &ft);
    snprintf(buf, sizeof buf, "Updated %02d:%02d", ft.tm_hour, ft.tm_min);
    put(buf, Layout::META_X, 404, &FreeSans9pt7b, Color::Black);
  }
}