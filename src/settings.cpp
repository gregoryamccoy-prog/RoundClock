#include "settings.h"

#include <Preferences.h>

#include "board_config.h"

namespace {
Preferences preferences;
}

void SettingsStore::begin() {
  preferences.begin("round-clock", false);
  settings_.use24Hour = preferences.getBool("24hour", false);
  settings_.fahrenheit = preferences.getBool("fahrenheit", config::kDefaultFahrenheit);
  settings_.brightness = preferences.getUChar("brightness", 220);
}

void SettingsStore::setUse24Hour(bool value) {
  if (settings_.use24Hour == value) return;
  settings_.use24Hour = value;
  preferences.putBool("24hour", value);
}

void SettingsStore::setBrightness(uint8_t value) {
  if (settings_.brightness == value) return;
  settings_.brightness = value;
  preferences.putUChar("brightness", value);
}

