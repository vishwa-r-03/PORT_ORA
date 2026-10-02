#include "ui.h"

#include <Arduino.h>

#include "display.h"
#include "ora_config.h"

namespace {

UI::Screen*       current    = nullptr;
volatile bool     dirty      = false;
volatile uint32_t dirtySince = 0;
uint32_t          lastRev    = 0;
TaskHandle_t      taskHandle = nullptr;

// All Display calls happen in this task. A refresh blocks for ~12-20 s.
void displayTask(void*) {
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(250));

    UI::Screen* s = current;
    if (!s) continue;

    const uint32_t rev = s->revision();
    if (rev != lastRev) {
      lastRev = rev;
      UI::markDirty();                                  // restarts the debounce window
    }

    if (!dirty) continue;
    if (millis() - dirtySince < UiCfg::DEBOUNCE_MS) continue;
    if (!Display::canRefresh()) continue;

    dirty = false;
    s->render();
    if (!Display::refresh()) UI::markDirty();           // retry later if it failed
  }
}

}  // namespace

namespace UI {

void begin() {
  if (taskHandle) return;
  xTaskCreatePinnedToCore(displayTask, "ui", 10240, nullptr, 1, &taskHandle, 1);
}

void show(Screen& s) {
  s.onEnter();
  lastRev = s.revision();
  current = &s;
  dirtySince = millis();      // the first frame also waits out the debounce, so a phone
  dirty = true;               // that reconnects at boot costs one flash instead of two
}

void markDirty() {
  dirtySince = millis();
  dirty = true;
}

}  // namespace UI