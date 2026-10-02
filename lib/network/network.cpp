#include "network.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>

#include "ora_config.h"
#include "secrets.h"

namespace Network {

bool connect(uint32_t timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  const uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - t0 > timeoutMs) {
      Serial.println("[net] wifi timeout (is the hotspot on, and set to 2.4 GHz?)");
      return false;
    }
    delay(200);
  }
  Serial.printf("[net] wifi ok, ip %s\n", WiFi.localIP().toString().c_str());
  return true;
}

void disconnect() {
  WiFi.disconnect(true, false);
  WiFi.mode(WIFI_OFF);
}

bool timeValid() {
  return time(nullptr) > 1735689600;              // after 2025-01-01
}

bool syncClock(uint32_t timeoutMs) {
  const bool alreadySet = timeValid();
  configTzTime(Locale::TZ, NetCfg::NTP_SERVER1, NetCfg::NTP_SERVER2);

  if (alreadySet) {                               // re-sync: replies usually arrive within a second
    delay(1500);
    return true;
  }

  const uint32_t t0 = millis();
  while (!timeValid()) {
    if (millis() - t0 > timeoutMs) {
      Serial.println("[net] ntp timeout");
      return false;
    }
    delay(100);
  }
  Serial.println("[net] clock set");
  return true;
}

bool httpsGet(const char* url, String& body, uint32_t timeoutMs) {
  WiFiClientSecure client;
  client.setInsecure();                           // no certificate check: fine for public weather data
  HTTPClient http;
  http.setTimeout((uint16_t)timeoutMs);
  if (!http.begin(client, url)) return false;

  const int code = http.GET();
  const bool ok = (code == HTTP_CODE_OK);
  if (ok) body = http.getString();
  else    Serial.printf("[net] http %d\n", code);
  http.end();
  return ok;
}

}  // namespace Network