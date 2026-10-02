#include "app.h"

#include <Arduino.h>

#include "bluetooth.h"
#include "ora_config.h"

namespace {

UI::Screen* nowPlayingScreen = nullptr;
UI::Screen* glanceScreen     = nullptr;
UI::Screen* active           = nullptr;
UI::Screen* pending          = nullptr;
uint32_t    pendingSince     = 0;

bool trackActive() {
  const Bluetooth::TrackInfo t = Bluetooth::snapshot();
  return t.connected && t.title[0] != '\0';
}

}  // namespace

namespace App {

void begin(UI::Screen& nowPlaying, UI::Screen& glance) {
  nowPlayingScreen = &nowPlaying;
  glanceScreen     = &glance;
}

void update() {
  if (!nowPlayingScreen || !glanceScreen) return;
  UI::Screen* desired = trackActive() ? nowPlayingScreen : glanceScreen;

  if (!active) {                                    // first screen: give the phone time to reconnect
    if (millis() < AppCfg::BOOT_GRACE_MS) return;
    active = desired;
    UI::show(*active);
    return;
  }

  if (desired == active) { pending = nullptr; return; }
  if (desired != pending) { pending = desired; pendingSince = millis(); return; }

  if (millis() - pendingSince >= AppCfg::MODE_SWITCH_MS) {
    active = desired;
    pending = nullptr;
    UI::show(*active);
  }
}

}  // namespace App