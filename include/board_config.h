#pragma once

#include <stdint.h>

namespace config {

// Waveshare 1.28-inch GC9A01 eight-pin display.
constexpr uint8_t kTftMosi = 23;
constexpr uint8_t kTftSclk = 18;
constexpr uint8_t kTftCs = 27;
constexpr uint8_t kTftDc = 26;
constexpr uint8_t kTftReset = 25;

// BL is wired to 3V3 for the recommended first build. Set this true only
// after moving BL to a transistor/logic-level driver controlled by GPIO32.
constexpr bool kBacklightPwmEnabled = false;
constexpr uint8_t kBacklightPin = 32;
constexpr uint8_t kButtonPin = 33;
constexpr bool kButtonEnabled = false;

constexpr uint16_t kScreenWidth = 240;
constexpr uint16_t kScreenHeight = 240;
constexpr uint8_t kDisplayRotation = 0;

constexpr double kLatitude = 38.3242;
constexpr double kLongitude = -85.4725;
constexpr char kLocationName[] = "CRESTWOOD";
// ESP32/newlib expects a POSIX TZ rule, not an IANA zone name.
constexpr char kTimezoneRule[] = "EST5EDT,M3.2.0/2,M11.1.0/2";
constexpr bool kDefaultFahrenheit = true;
constexpr bool kDefault24Hour = false;
constexpr uint32_t kWeatherRefreshMs = 15UL * 60UL * 1000UL;
constexpr uint32_t kWeatherStaleMs = 60UL * 60UL * 1000UL;
constexpr uint32_t kNtpSyncIntervalMs = 60UL * 60UL * 1000UL;

constexpr char kNtpServer1[] = "pool.ntp.org";
constexpr char kNtpServer2[] = "time.nist.gov";

}  // namespace config

