#pragma once
#include "ui.h"

class NowPlayingScreen : public UI::Screen {
 public:
  void render() override;
  uint32_t revision() const override;   // changes only when the visible content changes

 private:
  mutable uint32_t seenBtRev_  = 0xFFFFFFFFu;
  mutable uint32_t hash_       = 0;
  mutable uint32_t contentRev_ = 0;
};