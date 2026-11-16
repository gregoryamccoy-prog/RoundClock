#pragma once

#include <stdint.h>

#include "graphics.h"
#include "weather_data.h"

struct ClockFrame {
  bool timeValid = false;
  bool use24Hour = false;
  bool wifiConnected = false;
  bool syncStale = false;
  bool weatherStale = true;
  int hour = 0;
  int minute = 0;
  int second = 0;
  int utcHour = 0;
  int utcMinute = 0;
  int year = 0;
  int month = 0;
  int day = 0;
  int weekday = 0;  // 0=Sunday
  int dayOfYear = 0;
  int isoWeek = 0;
};

namespace palette {
constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}
constexpr uint16_t kBlack = rgb565(0, 4, 3);
constexpr uint16_t kOuterBlack = rgb565(8, 10, 10);
constexpr uint16_t kRingGreen = rgb565(0, 70, 51);
constexpr uint16_t kRingBright = rgb565(18, 121, 92);
constexpr uint16_t kMint = rgb565(140, 239, 209);
constexpr uint16_t kMintBright = rgb565(119, 255, 218);
constexpr uint16_t kActive = rgb565(6, 44, 36);
constexpr uint16_t kInactive = rgb565(119, 205, 180);
constexpr uint16_t kDivider = rgb565(8, 27, 22);
constexpr uint16_t kWhite = rgb565(241, 248, 245);
constexpr uint16_t kGrey = rgb565(77, 86, 84);
constexpr uint16_t kRed = rgb565(240, 50, 46);
}  // namespace palette

class FaceRenderer {
 public:
  void drawStaticFace(Graphics& g) const;
  void drawDial(Graphics& g, const ClockFrame& frame) const;
  void drawDialHands(Graphics& g, const ClockFrame& frame) const;
  void drawTimePanel(Graphics& g, const ClockFrame& frame) const;
  void drawCalendar(Graphics& g, const ClockFrame& frame) const;
  void drawWeather(Graphics& g, const WeatherData& weather,
                   const ClockFrame& frame, bool fahrenheit) const;
  void drawAllDynamic(Graphics& g, const ClockFrame& frame,
                      const WeatherData& weather, bool fahrenheit) const;

 private:
  static void drawText(Graphics& g, int x, int y, const char* text,
                       uint16_t color, uint8_t scale = 1);
  static void drawCenteredText(Graphics& g, int centerX, int y, const char* text,
                               uint16_t color, uint8_t scale = 1);
  static int textWidth(const char* text, uint8_t scale);
  static void drawSegmentDigit(Graphics& g, int x, int y, int w, int h,
                               char value, uint16_t active, uint16_t inactive);
  static void drawSegmentString(Graphics& g, int x, int y, int digitW,
                                int digitH, int gap, const char* text);
  static void drawWeatherIcon(Graphics& g, int cx, int cy, WeatherKind kind,
                              bool isDay, bool stale);
  static void drawWifiIcon(Graphics& g, int cx, int cy, bool connected);
};

