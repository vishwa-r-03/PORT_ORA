#include "sync.h"

#include <Arduino.h>

#include "network.h"
#include "ora_config.h"

namespace {

SemaphoreHandle_t mtx = nullptr;
Weather::Data     wx;
uint32_t          lastOkMs = 0;
bool              everOk   = false;
bool            (*deferCheck)() = nullptr;
TaskHandle_t      task = nullptr;

void ensureMutex() {
  if (!mtx) mtx = xSemaphoreCreateMutex();
}

void syncTask(void*) {
  uint32_t wait = NetCfg::SYNC_INTERVAL_MS;
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(wait));

    const bool stale = Sync::ageMs() > NetCfg::STALE_MS;
    if (deferCheck && deferCheck() && !stale) {
      wait = 60000;                                  // music playing: look again in a minute
      continue;
    }
    wait = Sync::runOnce() ? NetCfg::SYNC_INTERVAL_MS : NetCfg::RETRY_MS;
  }
}

}  // namespace

namespace Sync {

bool runOnce() {
  ensureMutex();
  Serial.printf("[sync] start, free heap %u, largest block %u\n",
                (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap());

  bool ok = false;
  if (Network::connect(NetCfg::WIFI_TIMEOUT_MS)) {
    const bool timeOk = Network::syncClock(NetCfg::NTP_TIMEOUT_MS);

    Weather::Data d;
    const bool wxOk = Weather::fetch(d);
    if (wxOk) {
      xSemaphoreTake(mtx, portMAX_DELAY);
      wx = d;
      xSemaphoreGive(mtx);
    }
    ok = timeOk || wxOk;
  }
  Network::disconnect();

  if (ok) { lastOkMs = millis(); everOk = true; }
  Serial.printf("[sync] %s\n", ok ? "ok" : "failed");
  return ok;
}

void startTask() {
  ensureMutex();
  if (task) return;
  xTaskCreatePinnedToCore(syncTask, "sync", 8192, nullptr, 1, &task, 1);
}

void setDeferCheck(bool (*fn)()) { deferCheck = fn; }

Weather::Data weather() {
  ensureMutex();
  xSemaphoreTake(mtx, portMAX_DELAY);
  Weather::Data copy = wx;
  xSemaphoreGive(mtx);
  return copy;
}

uint32_t ageMs() {
  return everOk ? (millis() - lastOkMs) : UINT32_MAX;
}

}  // namespace Sync