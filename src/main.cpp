#include <Arduino.h>

#include "app.h"
#include "bluetooth.h"
#include "display.h"
#include "glance.h"
#include "now_playing.h"
#include "ora_config.h"
#include "sync.h"
#include "ui.h"

static NowPlayingScreen nowPlaying;
static GlanceScreen     glance;

void setup() {
  Serial.begin(115200);
  Display::begin();

  Sync::runOnce();            // before Bluetooth: TLS needs a large free heap block
  Bluetooth::begin();

  Sync::setDeferCheck([]() {
    const Bluetooth::TrackInfo t = Bluetooth::snapshot();
    return t.connected && t.title[0] != '\0';
  });
  Sync::startTask();

  UI::begin();
  App::begin(nowPlaying, glance);
}

void loop() {
  App::update();
  delay(500);
}