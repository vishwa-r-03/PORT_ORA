// panel.h
#pragma once
#include <stdint.h>

namespace Panel {
  bool begin();                      // one-time GPIO / SPI / PWR pin setup
  bool wake();                       // init after power-up or sleep
  void show(const uint8_t* frame);   // push 24,960-byte frame + refresh; blocks
  void sleep();                      // deep sleep, PWR off
}