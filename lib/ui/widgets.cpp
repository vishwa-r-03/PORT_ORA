#include "widgets.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <stdio.h>
#include <string.h>

#include "ora_config.h"

using Display::Color;

namespace {

uint16_t measure(Adafruit_GFX& g, const char* s, int16_t baseline, int16_t* leftOffset = nullptr) {
  int16_t x1, y1;
  uint16_t w, h;
  g.getTextBounds(s, 0, baseline, &x1, &y1, &w, &h);
  if (leftOffset) *leftOffset = x1;
  return w;
}

void prepare(Adafruit_GFX& g, const GFXfont* font, Color c) {
  g.setTextSize(1);                 // Display::drawText may have left a larger size set
  g.setFont(font);
  g.setTextColor((uint16_t)c);
}

}  // namespace

namespace Widgets {

const GFXfont* titleFont()     { return &FreeSansBold12pt7b; }
const GFXfont* bodyFont()      { return &FreeSans9pt7b; }
const GFXfont* boldSmallFont() { return &FreeSansBold9pt7b; }

void drawFitted(const char* text, int16_t x, int16_t baseline, int16_t maxWidth,
                const GFXfont* font, const GFXfont* fallbackFont, Color color) {
  Adafruit_GFX& g = Display::canvas();
  const char* src = text ? text : "";
  prepare(g, font, color);

  if (fallbackFont && (int)measure(g, src, baseline) > maxWidth) {
    g.setFont(fallbackFont);
  }

  char buf[80];
  strlcpy(buf, src, sizeof buf);
  size_t len = strlen(buf);
  while (len > 1 && (int)measure(g, buf, baseline) > maxWidth) {
    len--;
    snprintf(buf, sizeof buf, "%.*s...", (int)len, src);   // GFX fonts have no ellipsis glyph
  }

  g.setCursor(x, baseline);
  g.print(buf);
  g.setFont(nullptr);
}

void drawRight(const char* text, int16_t rightX, int16_t baseline,
               const GFXfont* font, Color color) {
  Adafruit_GFX& g = Display::canvas();
  prepare(g, font, color);
  int16_t x1 = 0;
  const uint16_t w = measure(g, text, baseline, &x1);
  g.setCursor(rightX - (int16_t)w - x1, baseline);
  g.print(text);
  g.setFont(nullptr);
}

void drawStatusBar(const char* rightText) {
  Adafruit_GFX& g = Display::canvas();
  Display::fillRect(0, 0, Layout::W, Layout::STATUS_H, Color::Red);

  prepare(g, &FreeSansBold9pt7b, Color::White);
  g.setCursor(8, 18);
  g.print("Ora");
  g.drawFastHLine(11, 3, 9, (uint16_t)Color::White);       // macron over the O: tune by eye
  g.setFont(nullptr);

  if (rightText && rightText[0]) {
    drawRight(rightText, Layout::W - 8, 18, &FreeSans9pt7b, Color::White);
  }
}

void drawDivider(int16_t y) {
  Display::fillRect(Layout::META_X, y, Layout::W - 2 * Layout::META_X, 2, Color::Red);
}

void formatDuration(char* out, size_t n, uint32_t ms) {
  const uint32_t total = ms / 1000;
  snprintf(out, n, "%lu:%02lu", (unsigned long)(total / 60), (unsigned long)(total % 60));
}

}  // namespace Widgets