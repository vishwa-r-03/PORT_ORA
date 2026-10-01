# PORTA ORA

This is a portable high fidelity bluetooth DAC and an e ink display which shows weather, date & time and other very minimal displays. 
The whole device will be 3.7 inch in diameter, exactly the size of the display and will contain some buttons and a scrool wheel for navigation.

## what the device contains

* An ESP32 as the brain
* A 3.7 inch e-ink display by waveshare
* A multi purpose scroll wheel and two buttons.
* an amplifier and 12 bit DAC for high fidelity audio.
* a usbC port for charging and an audio jack port.

## File structure

```
ora-player/
├─ platformio.ini
├─ include/
│  ├─ config.h        # pins + constants
│  └─ secrets.h       # git-ignored (dev defaults only)
├─ lib/
│  ├─ display/        # display.h / display.cpp (+ vendor/ Waveshare driver)
│  ├─ input/          # encoder + buttons
│  ├─ audio/          # I2S + A2DP + SD playback
│  ├─ network/        # WiFi
│  ├─ provisioning/   # Bluetooth WiFi setup
│  └─ power/          # battery sense, sleep
└─ src/
   └─ main.cpp
|_ .gitignore 
```  



### What's done so far

- Tested the display and confirmed everything works.
- Planned out the product and its functionality.


The circuit diagrams and other updates will be added as the project progresses.