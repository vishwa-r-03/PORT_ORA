#pragma once
#include "ui.h"

class GlanceScreen : public UI::Screen {
 public:
  void render() override;
  uint32_t revision() const override;     // changes with the clock step or new weather

 private:
  mutable uint32_t hash_       = 0;
  mutable uint32_t contentRev_ = 0;
};