#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "Audio.h"
#include "ora_config.h"

static const char* kTrack = "/Tulasi_-_Sumedh_K_(Official_Music_Video).wav";   // change to your file name; a .wav works the same way

SPIClass sdSpi(HSPI);
Audio audio;

static void listRoot() {
  File root = SD.open("/");
  for (File f = root.openNextFile(); f; f = root.openNextFile()) {
    Serial.printf("  %s  (%u bytes)%s\n", f.name(), (unsigned)f.size(), f.isDirectory() ? "  [dir]" : "");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(Pins::SD_BENCH_CS, OUTPUT);
  digitalWrite(Pins::SD_BENCH_CS, HIGH);
  sdSpi.begin(Pins::SD_BENCH_SCK, Pins::SD_BENCH_MISO, Pins::SD_BENCH_MOSI, Pins::SD_BENCH_CS);

  if (!SD.begin(Pins::SD_BENCH_CS, sdSpi, 10000000)) {
    Serial.println("[sd] mount failed: check wiring, FAT32 format and the module's supply voltage");
    while (true) delay(1000);
  }
  Serial.printf("[sd] mounted, %llu MB\n", SD.cardSize() / (1024ULL * 1024ULL));
  listRoot();

  audio.setPinout(Pins::I2S_BCK, Pins::I2S_WS, Pins::I2S_DOUT);
  audio.setVolume(5);                          // 0..21, start low: the DAC output is hot
  audio.connecttoFS(SD, kTrack);
}

void loop() {
  audio.loop();                                // must run continuously, never block here

  while (Serial.available()) {                 // + / - volume, p = pause/resume
    const char c = Serial.read();
    if (c == '+')      audio.setVolume(min(21, audio.getVolume() + 1));
    else if (c == '-') audio.setVolume(max(0, (int)audio.getVolume() - 1));
    else if (c == 'p') audio.pauseResume();
  }
}

void audio_info(const char* info) { Serial.printf("[audio] %s\n", info); }
void audio_eof_mp3(const char* info) {
  Serial.printf("[audio] finished: %s\n", info);
  audio.connecttoFS(SD, kTrack);               // repeat
}