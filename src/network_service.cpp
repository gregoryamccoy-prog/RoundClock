#include "network_service.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_sntp.h>
#include <time.h>

#include <algorithm>
#include <cmath>

#include "board_config.h"
#include "core_logic.h"
#include "root_ca.h"
#include "secrets.h"
#include "weather_data.h"

namespace {
NetworkState* callbackState = nullptr;

void onTimeSync(struct timeval*) {
  if (!callbackState) return;
  callbackState->timeSynchronized.store(true);
  callbackState->lastNtpSyncMs.store(millis());
}

bool hasValidTime() {
  const time_t now = time(nullptr);
  return now > 1704067200;  // 2024-01-01, never use compile time as a fallback.
}

void formatLocalEpoch(time_t epoch, char result[6]) {
  if (epoch <= 0) {
    std::snprintf(result, 6, "--:--");
    return;
  }
  struct tm local {};
  localtime_r(&epoch, &local);
  std::snprintf(result, 6, "%02d:%02d", local.tm_hour, local.tm_min);
}
}  // namespace

void NetworkService::begin() {
  callbackState = &state_;
  xTaskCreatePinnedToCore(taskEntry, "network", 12288, this, 1, nullptr, 0);
}

void NetworkService::taskEntry(void* context) {
  static_cast<NetworkService*>(context)->taskLoop();
}

void NetworkService::taskLoop() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  bool ntpConfigured = false;
  uint32_t nextWifiAttempt = 0;
  uint32_t nextWeatherAttempt = 0;
  uint8_t weatherFailures = 0;

  for (;;) {
    const uint32_t nowMs = millis();
    const bool connected = WiFi.status() == WL_CONNECTED;
    state_.wifiConnected.store(connected);

    if (!connected && static_cast<int32_t>(nowMs - nextWifiAttempt) >= 0) {
      Serial.println("[network] connecting to Wi-Fi");
      WiFi.disconnect(false, false);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      nextWifiAttempt = nowMs + 10000;
    }

    if (connected && !ntpConfigured) {
      sntp_set_time_sync_notification_cb(onTimeSync);
      sntp_set_sync_interval(config::kNtpSyncIntervalMs);
      configTzTime(config::kTimezoneRule, config::kNtpServer1, config::kNtpServer2);
      ntpConfigured = true;
      nextWeatherAttempt = nowMs;
      Serial.print("[network] Wi-Fi connected, IP ");
      Serial.println(WiFi.localIP());
    }

    if (hasValidTime()) state_.timeSynchronized.store(true);

    if (connected && hasValidTime() &&
        static_cast<int32_t>(nowMs - nextWeatherAttempt) >= 0) {
      if (fetchWeather()) {
        weatherFailures = 0;
        nextWeatherAttempt = nowMs + config::kWeatherRefreshMs;
      } else {
        weatherFailures = std::min<uint8_t>(weatherFailures + 1, 4);
        const uint32_t retry = std::min<uint32_t>(60000UL << (weatherFailures - 1),
                                                  config::kWeatherRefreshMs);
        nextWeatherAttempt = nowMs + retry;
        Serial.printf("[weather] request failed; retry in %lu seconds\n", retry / 1000UL);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

bool NetworkService::fetchWeather() {
  char url[512];
  std::snprintf(
      url, sizeof(url),
      "https://api.open-meteo.com/v1/forecast?latitude=%.5f&longitude=%.5f"
      "&current=temperature_2m,relative_humidity_2m,weather_code,is_day"
      "&daily=sunrise,sunset&temperature_unit=%s&timeformat=unixtime"
      "&timezone=auto&forecast_days=2",
      config::kLatitude, config::kLongitude, fahrenheit_ ? "fahrenheit" : "celsius");

  WiFiClientSecure client;
  client.setCACert(kOpenMeteoRootCa);
  client.setTimeout(8);

  HTTPClient http;
  http.setConnectTimeout(6000);
  http.setTimeout(8000);
  http.useHTTP10(true);
  if (!http.begin(client, url)) return false;
  const int status = http.GET();
  if (status != HTTP_CODE_OK) {
    Serial.printf("[weather] HTTP status %d\n", status);
    http.end();
    return false;
  }
  const int contentLength = http.getSize();
  if (contentLength > 32768) {
    Serial.println("[weather] response exceeded 32 KiB limit");
    http.end();
    return false;
  }

  JsonDocument filter;
  filter["current"]["temperature_2m"] = true;
  filter["current"]["relative_humidity_2m"] = true;
  filter["current"]["weather_code"] = true;
  filter["current"]["is_day"] = true;
  filter["daily"]["sunrise"][0] = true;
  filter["daily"]["sunset"][0] = true;

  JsonDocument document;
  const DeserializationError error = deserializeJson(
      document, http.getStream(), DeserializationOption::Filter(filter),
      DeserializationOption::NestingLimit(8));
  http.end();
  if (error) {
    Serial.printf("[weather] JSON error: %s\n", error.c_str());
    return false;
  }

  const float temperature = document["current"]["temperature_2m"] | NAN;
  const int humidity = document["current"]["relative_humidity_2m"] | -1;
  const int code = document["current"]["weather_code"] | -1;
  if (!std::isfinite(temperature) || humidity < 0 || humidity > 100 || code < 0) {
    Serial.println("[weather] required fields were missing or invalid");
    return false;
  }

  WeatherData weather;
  weather.valid = true;
  weather.temperature = temperature;
  weather.humidity = static_cast<uint8_t>(humidity);
  weather.weatherCode = static_cast<int16_t>(code);
  weather.kind = core::weatherKindFromCode(code);
  weather.isDay = (document["current"]["is_day"] | 1) != 0;
  weather.receivedAtMs = millis();
  formatLocalEpoch(document["daily"]["sunrise"][0].as<time_t>(), weather.sunrise);
  formatLocalEpoch(document["daily"]["sunset"][0].as<time_t>(), weather.sunset);

  xQueueOverwrite(weatherQueue_, &weather);
  state_.weatherRevision.fetch_add(1);
  Serial.printf("[weather] %.1f degrees, %u%% RH, WMO %d, heap %u\n",
                weather.temperature, weather.humidity, weather.weatherCode,
                ESP.getFreeHeap());
  return true;
}

