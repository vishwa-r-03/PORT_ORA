#pragma once
#include <stdint.h>
#include "weather.h"

namespace Icons {

  // Draws a weather icon inside the size x size box whose top-left is (x, y).
  void drawWeather(int16_t x, int16_t y, int16_t size, Weather::Kind kind, bool isDay);

}