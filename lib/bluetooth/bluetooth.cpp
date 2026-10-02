#include "bluetooth.h"

#include <Arduino.h>
#include <BluetoothA2DPSink.h>
#include <stdlib.h>
#include <string.h>

#include "ora_config.h"

namespace {

BluetoothA2DPSink        a2dp;
SemaphoreHandle_t        mtx = nullptr;
Bluetooth::TrackInfo     cur{};
volatile uint32_t        rev = 0;

// Collapses runs of spaces/commas into ", " or " " and strips them from both ends,
// so tags like ", , ,Artist , , ," become "Artist".
void tidy(const char* in, char* out, size_t cap) {
  size_t o = 0;
  bool sep = false, comma = false;
  for (size_t i = 0; in[i]; i++) {
    const char c = in[i];
    if (c == ' ' || c == ',') {
      sep = true;
      if (c == ',') comma = true;
      continue;
    }
    if (sep && o > 0) {
      if (comma && o + 1 < cap) out[o++] = ',';
      if (o + 1 < cap)          out[o++] = ' ';
    }
    sep = comma = false;
    if (o + 1 < cap) out[o++] = c;
  }
  out[o] = '\0';
}

// Converts UTF-8 metadata to printable ASCII (each non-ASCII sequence becomes one
// '?', control characters become spaces), tidies it, and stores it in dst.
// Returns true if dst changed.
bool setField(char* dst, size_t n, const uint8_t* src) {
  char ascii[64];
  size_t o = 0;
  if (src) {
    for (size_t i = 0; src[i] && o < sizeof ascii - 1; i++) {
      const uint8_t c = src[i];
      if (c < 0x20)       ascii[o++] = ' ';
      else if (c < 0x80)  ascii[o++] = (char)c;
      else if (c >= 0xC0) ascii[o++] = '?';       // lead byte; continuation bytes skipped
    }
  }
  ascii[o] = '\0';

  char clean[64];
  tidy(ascii, clean, sizeof clean < n ? sizeof clean : n);

  if (strcmp(dst, clean) == 0) return false;
  strlcpy(dst, clean, n);
  return true;
}

void onMetadata(uint8_t id, const uint8_t* text) {
  if (Bt::LOG_METADATA) {
    Serial.printf("[bt] meta 0x%02X: %s\n", id, text ? (const char*)text : "");
  }
  if (!mtx) return;

  xSemaphoreTake(mtx, portMAX_DELAY);
  bool changed = false;
  switch (id) {
    case ESP_AVRC_MD_ATTR_TITLE:
      changed = setField(cur.title, sizeof cur.title, text);
      break;
    case ESP_AVRC_MD_ATTR_ARTIST:
      changed = setField(cur.artist, sizeof cur.artist, text);
      break;
    case ESP_AVRC_MD_ATTR_ALBUM:
      changed = setField(cur.album, sizeof cur.album, text);
      break;
    case ESP_AVRC_MD_ATTR_PLAYING_TIME: {
      const uint32_t d = text ? (uint32_t)atol((const char*)text) : 0;
      if (d != cur.durationMs) { cur.durationMs = d; changed = true; }
      break;
    }
    default:
      break;
  }
  if (changed) rev = rev + 1;
  xSemaphoreGive(mtx);
}

void onConnection(esp_a2d_connection_state_t state, void*) {
  if (!mtx) return;

  bool nowConnected;
  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED)         nowConnected = true;
  else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) nowConnected = false;
  else return;                                              // connecting / disconnecting

  xSemaphoreTake(mtx, portMAX_DELAY);
  if (cur.connected != nowConnected) {
    cur.connected = nowConnected;
    if (!nowConnected) {                                    // forget the old track
      cur.title[0] = cur.artist[0] = cur.album[0] = '\0';
      cur.durationMs = 0;
    }
    rev = rev + 1;
  }
  xSemaphoreGive(mtx);
  Serial.printf("[bt] %s\n", nowConnected ? "connected" : "disconnected");
}

}  // namespace

namespace Bluetooth {

void begin() {
  if (mtx) return;                                          // already started
  mtx = xSemaphoreCreateMutex();

  a2dp.set_avrc_metadata_callback(onMetadata);
  a2dp.set_on_connection_state_changed(onConnection);
  // If the track duration never arrives, request it explicitly. The parameter
  // type differs between library versions, so check the header first:
  // a2dp.set_avrc_metadata_attribute_mask(ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST |
  //                                       ESP_AVRC_MD_ATTR_ALBUM | ESP_AVRC_MD_ATTR_PLAYING_TIME);

  // Default output = internal DAC on GPIO25/26. Nothing needs to be attached.
  a2dp.start(Bt::DEVICE_NAME);
}

TrackInfo snapshot() {
  TrackInfo copy;
  if (mtx) xSemaphoreTake(mtx, portMAX_DELAY);
  copy = cur;
  if (mtx) xSemaphoreGive(mtx);
  return copy;
}

uint32_t revision() { return rev; }

}  // namespace Bluetooth