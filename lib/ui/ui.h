#pragma once
#include <stdint.h>

namespace UI {

  class Screen {
   public:
    virtual ~Screen() = default;
    virtual void render() = 0;                        // draws into the framebuffer only
    virtual void onEnter() {}
    virtual uint32_t revision() const { return 0; }   // bump when visible data changes
  };

  void begin();               // starts the display task
  void show(Screen& s);       // switch screen; rendered at the next opportunity
  void markDirty();           // request a refresh (debounced and rate-limited)

}