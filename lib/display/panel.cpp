#include "panel.h"
#include "vendor/DEV_Config.h"
#include "vendor/EPD_3in7g.h"

namespace Panel {

bool begin() {
  DEV_Module_Init();        // configures pins, powers the panel, starts Serial
  return true;
}

bool wake() {
  DEV_Module_Init();        // re-powers the panel after sleep() cut PWR
  EPD_3IN7G_Init();         // try EPD_3IN7G_Init_Fast() later for the ~12 s mode
  return true;
}

void show(const uint8_t* frame) {
  EPD_3IN7G_Display(frame);
}

void sleep() {
  EPD_3IN7G_Sleep();
  DEV_Delay_ms(2000);       // other Waveshare demos wait ~2 s here; check ESP32.ino
  DEV_Module_Exit();        // PWR low, RST low
}

}  // namespace Panel