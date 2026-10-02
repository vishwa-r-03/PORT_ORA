#include "now_playing.h"

#include <Arduino.h>

#include "art.h"
#include "bluetooth.h"
#include "display.h"
#include "ora_config.h"
#include "widgets.h"

using Display::Color;

namespace {

uint32_t fnv(uint32_t h, const char* s) {
  for (; *s; s++) { h ^= (uint8_t)*s; h *= 16777619u; }
  h ^= 0xFF;                                    // separator between fields
  h *= 16777619u;
  return h;
}

}  // namespace

// The Bluetooth revision moves on every metadata attribute; this hashes what is
// actually drawn, so a repeat of the same track (or any no-op) causes no refresh.
uint32_t NowPlayingScreen::revision() const {
  const uint32_t btRev = Bluetooth::revision();
  if (btRev == seenBtRev_) return contentRev_;
  seenBtRev_ = btRev;

  const Bluetooth::TrackInfo t = Bluetooth::snapshot();
  uint32_t h = 2166136261u;
  h = fnv(h, t.connected ? "1" : "0");
  h = fnv(h, t.title);
  h = fnv(h, t.artist);
  h = fnv(h, t.album);
  h ^= t.durationMs;
  h *= 16777619u;

  if (h != hash_) {
    hash_ = h;
    contentRev_++;
  }
  return contentRev_;
}

void NowPlayingScreen::render() {
  const Bluetooth::TrackInfo t = Bluetooth::snapshot();
  const bool hasTrack = t.connected && t.title[0] != '\0';

  Display::clear(Color::White);
  Widgets::drawStatusBar(!t.connected ? "Waiting" : (hasTrack ? "Bluetooth" : "Ready"));

  // ---- idle: nothing connected or nothing playing yet
  if (!hasTrack) {
    Art::drawGenerated(Layout::ART_X, Layout::ART_Y, Layout::ART_SIZE, Bt::DEVICE_NAME, "");
    Widgets::drawFitted(Bt::DEVICE_NAME, Layout::META_X, Layout::TITLE_BASELINE, Layout::TEXT_W,
                        Widgets::titleFont(), Widgets::bodyFont(), Color::Black);
    Widgets::drawFitted(t.connected ? "Play music on your phone" : "Pair via Bluetooth",
                        Layout::META_X, Layout::ARTIST_BASELINE, Layout::TEXT_W,
                        Widgets::bodyFont(), nullptr, Color::Red);
    return;
  }

  // ---- playing
  const char* seedA = t.album[0] ? t.album : t.title;
  Art::drawGenerated(Layout::ART_X, Layout::ART_Y, Layout::ART_SIZE, seedA, t.artist);

  Widgets::drawFitted(t.title, Layout::META_X, Layout::TITLE_BASELINE, Layout::TEXT_W,
                      Widgets::titleFont(), Widgets::bodyFont(), Color::Black);
  Widgets::drawFitted(t.artist[0] ? t.artist : "Unknown artist",
                      Layout::META_X, Layout::ARTIST_BASELINE, Layout::TEXT_W,
                      Widgets::bodyFont(), nullptr, Color::Red);
  if (t.album[0]) {
    Widgets::drawFitted(t.album, Layout::META_X, Layout::ALBUM_BASELINE, Layout::TEXT_W,
                        Widgets::bodyFont(), nullptr, Color::Black);
  }

  Widgets::drawDivider(Layout::FOOTER_RULE_Y);
  if (t.durationMs > 0) {
    char dur[16];
    Widgets::formatDuration(dur, sizeof dur, t.durationMs);
    Widgets::drawFitted(dur, Layout::META_X, Layout::FOOTER_BASELINE, Layout::TEXT_W,
                        Widgets::bodyFont(), nullptr, Color::Black);
  }
  // TODO: battery percentage once the power module exists
}