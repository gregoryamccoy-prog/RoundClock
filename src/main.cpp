#include <Arduino.h>
#include <time.h>

#include "board_config.h"
#include "core_logic.h"
#include "display_service.h"
#include "network_service.h"
#include "settings.h"
#include "weather_data.h"

namespace {
SettingsStore settingsStore;
DisplayService display;
NetworkState networkState;
QueueHandle_t weatherQueue = nullptr;
NetworkService* network = nullptr;
WeatherData currentWeather;

ClockFrame makeClockFrame() {
  ClockFrame frame;
  frame.use24Hour = settingsStore.get().use24Hour;
  frame.wifiConnected = networkState.wifiConnected.load();

  const time_t now = time(nullptr);
  frame.timeValid = now > 1704067200;
  if (frame.timeValid) {
    struct tm local {};
    struct tm utc {};
    localtime_r(&now, &local);
    gmtime_r(&now, &utc);
    frame.hour = local.tm_hour;
    frame.minute = local.tm_min;
    frame.second = local.tm_sec;
    frame.utcHour = utc.tm_hour;
    frame.utcMinute = utc.tm_min;
    frame.year = local.tm_year + 1900;
    frame.month = local.tm_mon + 1;
    frame.day = local.tm_mday;
    frame.weekday = local.tm_wday;
    frame.dayOfYear = local.tm_yday + 1;
    frame.isoWeek = core::isoWeekNumber(frame.year, frame.month, frame.day);
  }

  const uint32_t lastSync = networkState.lastNtpSyncMs.load();
  frame.syncStale = frame.timeValid &&
      (lastSync == 0 || millis() - lastSync > 2UL * config::kNtpSyncIntervalMs);
  frame.weatherStale = !currentWeather.valid ||
      millis() - currentWeather.receivedAtMs > config::kWeatherStaleMs;
  return frame;
}

void applyBrightness(uint8_t brightness) {
  if (!config::kBacklightPwmEnabled) return;
  ledcWrite(0, brightness);
}

void handleButton() {
  if (!config::kButtonEnabled) return;
  static bool previousPressed = false;
  static uint32_t pressedAt = 0;
  static uint32_t lastTransition = 0;
  const bool pressed = digitalRead(config::kButtonPin) == LOW;
  const uint32_t now = millis();
  if (pressed != previousPressed && now - lastTransition >= 30) {
    lastTransition = now;
    previousPressed = pressed;
    if (pressed) {
      pressedAt = now;
    } else {
      const uint32_t duration = now - pressedAt;
      if (duration >= 900) {
        settingsStore.setUse24Hour(!settingsStore.get().use24Hour);
      } else if (duration >= 40 && config::kBacklightPwmEnabled) {
        const uint8_t current = settingsStore.get().brightness;
        const uint8_t next = current > 210 ? 150 : (current > 110 ? 70 : 255);
        settingsStore.setBrightness(next);
        applyBrightness(next);
      }
    }
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nRoundClock starting");

  // Apply local/DST conversion immediately, including after a warm reset where
  // the ESP32 may already have a valid system clock before Wi-Fi reconnects.
  setenv("TZ", config::kTimezoneRule, 1);
  tzset();

  settingsStore.begin();
  if (config::kButtonEnabled) pinMode(config::kButtonPin, INPUT_PULLUP);
  if (config::kBacklightPwmEnabled) {
    ledcSetup(0, 5000, 8);
    ledcAttachPin(config::kBacklightPin, 0);
    applyBrightness(settingsStore.get().brightness);
  }

  weatherQueue = xQueueCreate(1, sizeof(WeatherData));
  if (!weatherQueue) {
    Serial.println("FATAL: unable to allocate weather queue");
    while (true) delay(1000);
  }

  const ClockFrame initialFrame = makeClockFrame();
  display.begin(initialFrame, currentWeather, settingsStore.get().fahrenheit);

  network = new NetworkService(weatherQueue, networkState,
                               settingsStore.get().fahrenheit);
  network->begin();
}

void loop() {
  handleButton();

  WeatherData incoming;
  bool receivedWeather = false;
  if (xQueueReceive(weatherQueue, &incoming, 0) == pdTRUE) {
    currentWeather = incoming;
    receivedWeather = true;
  }

  static int64_t previousSecond = -1;
  static int previousDateKey = -1;
  static int previousMinuteKey = -1;
  static bool previousWeatherStale = true;
  static bool previous24Hour = settingsStore.get().use24Hour;

  const time_t now = time(nullptr);
  const int64_t secondKey = now > 1704067200 ? static_cast<int64_t>(now) : millis() / 1000;
  if (secondKey != previousSecond) {
    previousSecond = secondKey;
    const ClockFrame frame = makeClockFrame();
    const int dateKey = frame.timeValid
        ? frame.year * 10000 + frame.month * 100 + frame.day : 0;
    const int minuteKey = frame.timeValid ? frame.utcHour * 60 + frame.utcMinute : -1;
    const bool modeChanged = previous24Hour != frame.use24Hour;
    const bool calendarChanged = dateKey != previousDateKey;
    const bool weatherChanged = receivedWeather || minuteKey != previousMinuteKey ||
                                previousWeatherStale != frame.weatherStale;
    display.update(frame, currentWeather, settingsStore.get().fahrenheit,
                   calendarChanged, weatherChanged);
    previousDateKey = dateKey;
    previousMinuteKey = minuteKey;
    previousWeatherStale = frame.weatherStale;
    previous24Hour = frame.use24Hour;
    (void)modeChanged;  // Time panel is redrawn every second regardless.
  }
  delay(10);
}

