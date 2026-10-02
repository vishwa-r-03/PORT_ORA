#include <Arduino.h>

#include "bluetooth.h"
#include "display.h"
#include "now_playing.h"
#include "ora_config.h"
#include "ui.h"

static NowPlayingScreen nowPlaying;

void setup() {
  Serial.begin(115200);
  Display::begin();
  Bluetooth::begin();
  UI::begin();
  UI::show(nowPlaying);
}

void loop() {
  delay(1000);
}