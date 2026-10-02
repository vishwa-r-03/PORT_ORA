#pragma once
#include "ui.h"

namespace App {

  void begin(UI::Screen& nowPlaying, UI::Screen& glance);
  void update();     // call regularly from loop()

}