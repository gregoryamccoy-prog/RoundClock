#pragma once

#include <stdint.h>

enum class WeatherKind : uint8_t {
  kUnknown,
  kClear,
  kPartlyCloudy,
  kCloudy,
  kFog,
  kRain,
  kSnow,
  kThunder,
};

struct WeatherData {
  bool valid = false;
  float temperature = 0.0f;
  uint8_t humidity = 0;
  int16_t weatherCode = -1;
  WeatherKind kind = WeatherKind::kUnknown;
  bool isDay = true;
  char sunrise[6] = "--:--";
  char sunset[6] = "--:--";
  uint32_t receivedAtMs = 0;
};

