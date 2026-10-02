#include "weather.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdio.h>

#include "network.h"
#include "ora_config.h"
#include "secrets.h"

namespace Weather {

bool fetch(Data& out) {
  if (WEATHER_LAT == 0.0 && WEATHER_LON == 0.0) {
    Serial.println("[wx] set WEATHER_LAT / WEATHER_LON in secrets.h");
    return false;
  }

  char url[300];
  snprintf(url, sizeof url,
           "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
           "&current=temperature_2m,weather_code,is_day"
           "&daily=temperature_2m_max,temperature_2m_min"
           "&timezone=auto&forecast_days=1",
           (double)WEATHER_LAT, (double)WEATHER_LON);

  String body;
  if (!Network::httpsGet(url, body, NetCfg::HTTP_TIMEOUT_MS)) return false;

  JsonDocument doc;
  if (deserializeJson(doc, body)) {
    Serial.println("[wx] bad json");
    return false;
  }

  JsonObject cur = doc["current"];
  if (cur.isNull()) return false;

  Data d;
  d.tempC = cur["temperature_2m"] | 0.0f;
  d.code  = cur["weather_code"]   | -1;
  d.isDay = (cur["is_day"] | 1) != 0;
  d.highC = doc["daily"]["temperature_2m_max"][0] | d.tempC;
  d.lowC  = doc["daily"]["temperature_2m_min"][0] | d.tempC;
  d.fetchedAt = time(nullptr);
  d.valid = true;

  out = d;
  Serial.printf("[wx] %.1fC code %d (H %.0f / L %.0f)\n", d.tempC, d.code, d.highC, d.lowC);
  return true;
}

Kind kind(int c) {
  if (c == 0)                       return Kind::Clear;
  if (c == 1 || c == 2)             return Kind::PartlyCloudy;
  if (c == 3)                       return Kind::Cloudy;
  if (c == 45 || c == 48)           return Kind::Fog;
  if ((c >= 51 && c <= 67) || (c >= 80 && c <= 82)) return Kind::Rain;
  if ((c >= 71 && c <= 77) || c == 85 || c == 86)   return Kind::Snow;
  if (c >= 95 && c <= 99)           return Kind::Storm;
  return Kind::Unknown;
}

const char* describe(int c) {
  switch (kind(c)) {
    case Kind::Clear:        return "Clear";
    case Kind::PartlyCloudy: return "Partly cloudy";
    case Kind::Cloudy:       return "Overcast";
    case Kind::Fog:          return "Fog";
    case Kind::Rain:         return (c >= 80) ? "Showers" : "Rain";
    case Kind::Snow:         return "Snow";
    case Kind::Storm:        return "Thunderstorm";
    default:                 return "Unknown";
  }
}

}  // namespace Weather