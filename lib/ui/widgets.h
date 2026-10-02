#pragma once
#include <stddef.h>
#include <stdint.h>
#include <gfxfont.h>

#include "display.h"

namespace Widgets {

  // Fonts live here so only one copy ends up in flash.
  const GFXfont* titleFont();       // bold 12 pt
  const GFXfont* bodyFont();        // regular 9 pt
  const GFXfont* boldSmallFont();   // bold 9 pt

  // Draws text left-aligned at baseline y. Steps down to fallbackFont (may be
  // nullptr) if too wide, then truncates with "...".
  void drawFitted(const char* text, int16_t x, int16_t baseline, int16_t maxWidth,
                  const GFXfont* font, const GFXfont* fallbackFont, Display::Color color);

  void drawRight(const char* text, int16_t rightX, int16_t baseline,
                 const GFXfont* font, Display::Color color);

  void drawStatusBar(const char* rightText);    // red strip, "Ora" left, text right
  void drawDivider(int16_t y);
  void formatDuration(char* out, size_t n, uint32_t ms);   // "m:ss"

}