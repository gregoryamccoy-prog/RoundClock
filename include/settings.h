#pragma once

#include <stdint.h>

struct AppSettings {
  bool use24Hour = false;
  bool fahrenheit = true;
  uint8_t brightness = 220;
};

class SettingsStore {
 public:
  void begin();
  const AppSettings& get() const { return settings_; }
  void setUse24Hour(bool value);
  void setBrightness(uint8_t value);

 private:
  AppSettings settings_;
};

