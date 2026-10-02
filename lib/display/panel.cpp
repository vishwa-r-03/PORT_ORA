#include "panel.h"

#include "ora_config.h"
#include "vendor/DEV_Config.h"
#include "vendor/EPD_3in7g.h"

namespace Panel {

bool begin() {
  DEV_Module_Init();        // configures pins, powers the panel, starts Serial
  return true;
}

bool wake() {
  DEV_Module_Init();        // re-powers the panel after sleep() cut PWR
  if (DisplayCfg::FAST_REFRESH) EPD_3IN7G_Init_Fast();
  else                          EPD_3IN7G_Init();
  return true;
}

void show(const uint8_t* frame) {
  // TODO: check ESP32.ino for which Display function the demo pairs with
  // Init_Fast (EPD_3IN7G_Display or EPD_3IN7G_Display_1) and use that one here.
  EPD_3IN7G_Display(frame);
}

void sleep() {
  EPD_3IN7G_Sleep();
  DEV_Delay_ms(2000);       // other Waveshare demos wait ~2 s here
  DEV_Module_Exit();        // PWR low, RST low
}

}  // namespace Panel