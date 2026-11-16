#include "face_renderer.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "core_logic.h"

namespace {
constexpr float kPi = 3.14159265358979323846f;

const uint8_t* glyph(char c) {
  // Five columns, least-significant bit at the top. Deliberately compact and
  // deterministic so host emulation and the physical screen are identical.
  static const uint8_t blank[5] = {0, 0, 0, 0, 0};
  static const uint8_t digits[10][5] = {
      {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},
      {0x62,0x51,0x49,0x49,0x46},{0x22,0x49,0x49,0x49,0x36},
      {0x18,0x14,0x12,0x7F,0x10},{0x2F,0x49,0x49,0x49,0x31},
      {0x3E,0x49,0x49,0x49,0x32},{0x01,0x71,0x09,0x05,0x03},
      {0x36,0x49,0x49,0x49,0x36},{0x26,0x49,0x49,0x49,0x3E}};
  static const uint8_t letters[26][5] = {
      {0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},
      {0x3E,0x41,0x41,0x41,0x22},{0x7F,0x41,0x41,0x22,0x1C},
      {0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},
      {0x3E,0x41,0x49,0x49,0x7A},{0x7F,0x08,0x08,0x08,0x7F},
      {0x00,0x41,0x7F,0x41,0x00},{0x20,0x40,0x41,0x3F,0x01},
      {0x7F,0x08,0x14,0x22,0x41},{0x7F,0x40,0x40,0x40,0x40},
      {0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},
      {0x3E,0x41,0x41,0x41,0x3E},{0x7F,0x09,0x09,0x09,0x06},
      {0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},
      {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7F,0x01,0x01},
      {0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},
      {0x3F,0x40,0x38,0x40,0x3F},{0x63,0x14,0x08,0x14,0x63},
      {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43}};
  static const uint8_t colon[5] = {0x00,0x36,0x36,0x00,0x00};
  static const uint8_t dash[5] = {0x08,0x08,0x08,0x08,0x08};
  static const uint8_t slash[5] = {0x20,0x10,0x08,0x04,0x02};
  static const uint8_t percent[5] = {0x23,0x13,0x08,0x64,0x62};
  static const uint8_t dot[5] = {0,0x60,0x60,0,0};
  if (c >= '0' && c <= '9') return digits[c - '0'];
  if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
  if (c >= 'A' && c <= 'Z') return letters[c - 'A'];
  if (c == ':') return colon;
  if (c == '-') return dash;
  if (c == '/') return slash;
  if (c == '%') return percent;
  if (c == '.') return dot;
  return blank;
}

void thickLine(Graphics& g, int x0, int y0, int x1, int y1,
               uint16_t color, int thickness) {
  for (int d = -thickness / 2; d <= thickness / 2; ++d) {
    g.drawLine(x0 + d, y0, x1 + d, y1, color);
    if (d != 0) g.drawLine(x0, y0 + d, x1, y1 + d, color);
  }
}

void drawRotatedText(Graphics& g, int centerX, int centerY, const char* text,
                     uint16_t color, float angle, int scale = 1) {
  const int length = static_cast<int>(std::strlen(text));
  const float textWidth = static_cast<float>(length * 6 * scale - scale);
  const float textHeight = static_cast<float>(7 * scale);
  const float cosine = std::cos(angle);
  const float sine = std::sin(angle);
  for (int index = 0; index < length; ++index) {
    const uint8_t* columns = glyph(text[index]);
    for (int col = 0; col < 5; ++col) {
      for (int row = 0; row < 7; ++row) {
        if (!((columns[col] >> row) & 1U)) continue;
        for (int sx = 0; sx < scale; ++sx) {
          for (int sy = 0; sy < scale; ++sy) {
            const float localX = index * 6 * scale + col * scale + sx - textWidth / 2.0f;
            const float localY = row * scale + sy - textHeight / 2.0f;
            const int x = centerX + static_cast<int>(std::lround(localX * cosine - localY * sine));
            const int y = centerY + static_cast<int>(std::lround(localX * sine + localY * cosine));
            g.drawPixel(x, y, color);
          }
        }
      }
    }
  }
}

}  // namespace

int FaceRenderer::textWidth(const char* text, uint8_t scale) {
  if (!text || !*text) return 0;
  return static_cast<int>(std::strlen(text)) * 6 * scale - scale;
}

void FaceRenderer::drawText(Graphics& g, int x, int y, const char* text,
                            uint16_t color, uint8_t scale) {
  if (!text) return;
  for (const char* p = text; *p; ++p, x += 6 * scale) {
    const uint8_t* columns = glyph(*p);
    for (int col = 0; col < 5; ++col) {
      for (int row = 0; row < 7; ++row) {
        if ((columns[col] >> row) & 1U) {
          g.fillRect(x + col * scale, y + row * scale, scale, scale, color);
        }
      }
    }
  }
}

void FaceRenderer::drawCenteredText(Graphics& g, int centerX, int y,
                                    const char* text, uint16_t color,
                                    uint8_t scale) {
  drawText(g, centerX - textWidth(text, scale) / 2, y, text, color, scale);
}

void FaceRenderer::drawStaticFace(Graphics& g) const {
  using namespace palette;
  g.fillScreen(kBlack);
  g.fillCircle(120, 120, 119, kOuterBlack);
  g.drawCircle(120, 120, 118, kGrey);
  g.drawCircle(120, 120, 117, kGrey);
  g.fillCircle(120, 120, 108, kOuterBlack);
  g.drawCircle(120, 120, 108, kGrey);
  g.fillCircle(120, 120, 102, kRingGreen);
  g.drawCircle(120, 120, 102, kBlack);
  g.fillCircle(120, 120, 84, kDivider);
  g.drawCircle(120, 120, 85, kBlack);

  // Full reference-style city and 24-hour rings. Both use tangent text, which
  // is essential to the original world-time instrument silhouette.
  static const char* cities[24] = {
      "AZO","DKR","LON","PAR","CAI","MOW","DUB","KAR",
      "DAC","BKK","KUL","SYS","HNL","SYD","AKL","HAW",
      "MHI","ANC","LAA","DEN","MEX","NYC","CCS","RIO"};
  for (int index = 0; index < 24; ++index) {
    const float radial = -kPi / 2.0f + index * kPi / 12.0f;
    const int cityX = 120 + static_cast<int>(std::lround(std::cos(radial) * 112.0f));
    const int cityY = 120 + static_cast<int>(std::lround(std::sin(radial) * 112.0f));
    drawRotatedText(g, cityX, cityY, cities[index], kWhite, radial + kPi / 2.0f);

    char hour[3];
    const int value = index == 0 ? 24 : index;
    std::snprintf(hour, sizeof(hour), "%d", value);
    const int hourX = 120 + static_cast<int>(std::lround(std::cos(radial) * 94.0f));
    const int hourY = 120 + static_cast<int>(std::lround(std::sin(radial) * 94.0f));
    drawRotatedText(g, hourX, hourY, hour, kMintBright, radial + kPi / 2.0f,
                    value < 10 ? 2 : 1);
  }

  // Upper temperature/humidity wedges.
  g.fillRect(42, 53, 49, 20, kMint);
  g.fillTriangle(42, 53, 42, 73, 35, 73, kMint);
  g.fillRect(149, 53, 49, 20, kMint);
  g.fillTriangle(198, 53, 205, 73, 198, 73, kMint);
  g.drawFastHLine(35, 74, 170, kBlack);

  // Sunrise/sunset wedges and lower weather/UTC blocks.
  g.fillRect(33, 76, 58, 19, kMint);
  g.fillTriangle(33, 76, 33, 95, 29, 95, kMint);
  g.fillRect(149, 76, 58, 19, kMint);
  g.fillTriangle(207, 76, 211, 95, 207, 95, kMint);
  g.fillRect(44, 97, 44, 31, kMint);
  g.fillRect(152, 97, 44, 31, kMint);

  // PB/WB decorative level bars from the accepted artwork.
  g.fillRect(36, 101, 4, 25, kMintBright);
  g.fillRect(200, 101, 4, 25, kMintBright);
  for (int y = 99; y <= 127; y += 6) {
    g.drawFastHLine(32, y, 4, kWhite);
    g.drawFastHLine(205, y, 4, kWhite);
  }
  drawText(g, 31, 128, "PB", kWhite);
  drawText(g, 199, 128, "WB", kWhite);

  // Calendar, large local time, year cassette, and brand line.
  g.fillRect(36, 132, 168, 14, kMint);
  g.fillTriangle(36, 132, 36, 146, 32, 146, kMint);
  g.fillTriangle(204, 132, 208, 146, 204, 146, kMint);
  g.drawFastVLine(85, 132, 14, kBlack);
  g.drawFastVLine(152, 132, 14, kBlack);
  g.fillRect(49, 148, 142, 30, kMint);
  g.fillTriangle(49, 148, 49, 178, 44, 169, kMint);
  g.fillTriangle(191, 148, 196, 169, 191, 178, kMint);
  g.drawFastHLine(49, 147, 142, kBlack);
  g.fillRect(70, 180, 100, 11, kMint);
  g.fillRect(70, 180, 24, 4, kRingBright);
  g.fillRect(146, 180, 24, 4, kRingBright);
  // The striped medallion is one of the strongest visual anchors in the art.
  g.fillCircle(120, 48, 19, kBlack);
  g.fillCircle(120, 48, 16, kMintBright);
  g.drawCircle(120, 48, 14, kActive);
  for (int stripe = -3; stripe <= 3; ++stripe) {
    int previousX = 120 + stripe;
    int previousY = 36;
    for (int step = 1; step <= 24; ++step) {
      const float t = step / 24.0f;
      const int y = 36 + step;
      const float bow = std::sin(t * kPi) * (stripe * 1.4f);
      const int x = 120 + stripe + static_cast<int>(std::lround((t - 0.5f) * stripe * 2.2f + bow));
      g.drawLine(previousX, previousY, x, y, kActive);
      previousX = x;
      previousY = y;
    }
  }

  drawCenteredText(g, 91, 32, "DY", kWhite);
  drawCenteredText(g, 149, 32, "WY", kWhite);
  drawCenteredText(g, 60, 136, "MYT", kActive);
}

void FaceRenderer::drawSegmentDigit(Graphics& g, int x, int y, int w, int h,
                                    char value, uint16_t active,
                                    uint16_t inactive) {
  const int t = w >= 15 ? 3 : 2;
  const int half = h / 2;
  const uint8_t mask = core::sevenSegmentMask(value);
  auto horizontal = [&](int yy, uint16_t c) {
    g.fillRect(x + t, yy, w - 2 * t, t, c);
    g.fillTriangle(x, yy + t / 2, x + t, yy, x + t, yy + t - 1, c);
    g.fillTriangle(x + w - 1, yy + t / 2, x + w - t - 1, yy,
                   x + w - t - 1, yy + t - 1, c);
  };
  auto vertical = [&](int xx, int yy, uint16_t c) {
    const int length = half - t;
    g.fillRect(xx, yy + t, t, length - 2 * t, c);
    g.fillTriangle(xx + t / 2, yy, xx, yy + t, xx + t - 1, yy + t, c);
    g.fillTriangle(xx + t / 2, yy + length, xx, yy + length - t,
                   xx + t - 1, yy + length - t, c);
  };
  auto draw = [&](uint16_t c, uint8_t bits) {
    if (bits & 0x01) horizontal(y, c);             // A
    if (bits & 0x02) vertical(x + w - t, y, c);    // B
    if (bits & 0x04) vertical(x + w - t, y + half, c); // C
    if (bits & 0x08) horizontal(y + h - t, c);     // D
    if (bits & 0x10) vertical(x, y + half, c);      // E
    if (bits & 0x20) vertical(x, y, c);             // F
    if (bits & 0x40) horizontal(y + half - t / 2, c); // G
  };
  draw(inactive, 0x7F);
  draw(active, mask);
}

void FaceRenderer::drawSegmentString(Graphics& g, int x, int y, int digitW,
                                     int digitH, int gap, const char* text) {
  for (const char* p = text; *p; ++p) {
    if (*p == ':') {
      const int r = digitW >= 15 ? 2 : 1;
      g.fillCircle(x + r, y + digitH / 3, r, palette::kActive);
      g.fillCircle(x + r, y + 2 * digitH / 3, r, palette::kActive);
      x += 2 * r + gap;
    } else {
      drawSegmentDigit(g, x, y, digitW, digitH, *p,
                       palette::kActive, palette::kInactive);
      x += digitW + gap;
    }
  }
}

void FaceRenderer::drawDial(Graphics& g, const ClockFrame& frame) const {
  using namespace palette;
  g.fillRect(91, 72, 58, 58, kDivider);
  g.drawRect(91, 72, 58, 58, kBlack);
  g.fillCircle(96, 77, 3, kBlack);
  g.drawCircle(96, 77, 2, kRingBright);
  g.fillCircle(144, 77, 3, kBlack);
  g.drawCircle(144, 77, 2, kRingBright);
  g.fillCircle(96, 125, 3, kBlack);
  g.drawCircle(96, 125, 2, kRingBright);
  g.fillCircle(144, 125, 3, kBlack);
  g.drawCircle(144, 125, 2, kRingBright);
  g.fillCircle(120, 100, 27, kBlack);
  g.fillCircle(120, 100, 17, kMint);
  g.drawCircle(120, 100, 18, kWhite);
  for (int i = 0; i < 60; ++i) {
    const float a = (i * 6.0f - 90.0f) * kPi / 180.0f;
    const int outer = 26;
    const int inner = (i % 5 == 0) ? 22 : 24;
    g.drawLine(120 + static_cast<int>(std::cos(a) * inner),
               100 + static_cast<int>(std::sin(a) * inner),
               120 + static_cast<int>(std::cos(a) * outer),
               100 + static_cast<int>(std::sin(a) * outer), kWhite);
  }
  static const char* dialLabels[12] = {"60","5","10","15","20","25","30","35","40","45","50","55"};
  for (int index = 0; index < 12; ++index) {
    const float a = (index * 30.0f - 90.0f) * kPi / 180.0f;
    const int x = 120 + static_cast<int>(std::lround(std::cos(a) * 21.0f));
    const int y = 100 + static_cast<int>(std::lround(std::sin(a) * 21.0f));
    drawCenteredText(g, x, y - 3, dialLabels[index], kWhite);
  }
  g.drawCircle(120, 100, 9, kActive);
  g.drawFastHLine(103, 100, 35, kActive);
  g.drawFastVLine(120, 83, 35, kActive);
  drawDialHands(g, frame);
}

void FaceRenderer::drawDialHands(Graphics& g, const ClockFrame& frame) const {
  using namespace palette;
  if (!frame.timeValid) {
    drawCenteredText(g, 120, 97, "SYNC", kActive);
    return;
  }
  const float secondA = (frame.second * 6.0f - 90.0f) * kPi / 180.0f;
  const float minuteA = ((frame.minute + frame.second / 60.0f) * 6.0f - 90.0f) * kPi / 180.0f;
  const float hourA = (((frame.hour % 12) + frame.minute / 60.0f) * 30.0f - 90.0f) * kPi / 180.0f;
  thickLine(g, 120, 100, 120 + static_cast<int>(std::cos(hourA) * 11),
            100 + static_cast<int>(std::sin(hourA) * 11), kActive, 3);
  thickLine(g, 120, 100, 120 + static_cast<int>(std::cos(minuteA) * 16),
            100 + static_cast<int>(std::sin(minuteA) * 16), kActive, 2);
  g.drawLine(120, 100, 120 + static_cast<int>(std::cos(secondA) * 17),
             100 + static_cast<int>(std::sin(secondA) * 17), kRed);
  g.fillCircle(120, 100, 4, kActive);
  g.fillCircle(120, 100, 2, kMintBright);
}

void FaceRenderer::drawWifiIcon(Graphics& g, int cx, int cy,
                                bool connected) {
  const uint16_t c = connected ? palette::kActive : palette::kGrey;
  // Three clean chevrons read better than tiny anti-aliased arcs on a 240px LCD.
  g.drawLine(cx - 8, cy - 4, cx, cy - 8, c);
  g.drawLine(cx, cy - 8, cx + 8, cy - 4, c);
  g.drawLine(cx - 5, cy, cx, cy - 3, c);
  g.drawLine(cx, cy - 3, cx + 5, cy, c);
  g.fillCircle(cx, cy + 4, 2, c);
  if (!connected) g.drawLine(cx - 8, cy - 8, cx + 8, cy + 6, palette::kActive);
}

void FaceRenderer::drawTimePanel(Graphics& g, const ClockFrame& frame) const {
  using namespace palette;
  g.fillRect(49, 148, 142, 30, kMint);
  if (!frame.timeValid) {
    drawSegmentString(g, 76, 150, 15, 26, 2, "--:--");
    drawText(g, 52, 154, "SYNC", kActive);
  } else {
    int displayHour = frame.hour;
    const bool pm = displayHour >= 12;
    if (!frame.use24Hour) {
      displayHour %= 12;
      if (displayHour == 0) displayHour = 12;
    }
    char hhmm[6];
    std::snprintf(hhmm, sizeof(hhmm), "%02d:%02d", displayHour, frame.minute);
    drawSegmentString(g, 76, 150, 15, 26, 2, hhmm);
    if (!frame.use24Hour) drawText(g, 52, 154, pm ? "PM" : "AM", kActive);
    char seconds[3];
    core::formatTwoDigits(frame.second, seconds);
    drawSegmentString(g, 158, 154, 7, 16, 1, seconds);
  }
  drawWifiIcon(g, 181, 160, frame.wifiConnected);
  if (frame.syncStale) drawText(g, 52, 169, "NTP", kActive);
}

void FaceRenderer::drawCalendar(Graphics& g, const ClockFrame& frame) const {
  using namespace palette;
  // Preserve the anti-aliased MYT label and cassette edges from the static
  // artwork; only restore the two genuinely dynamic subfields.
  g.fillRect(86, 133, 65, 12, kMint);
  g.fillRect(153, 133, 50, 12, kMint);
  static const char* days[] = {"S", "M", "T", "W", "T", "F", "S"};
  for (int i = 0; i < 7; ++i) {
    const int cx = 91 + i * 9;
    if (frame.timeValid && i == frame.weekday) {
      g.fillRect(cx - 3, 133, 7, 12, kActive);
      drawCenteredText(g, cx, 135, days[i], kMintBright);
    } else {
      drawCenteredText(g, cx, 135, days[i], kActive);
    }
  }
  char date[6] = "--/--";
  if (frame.timeValid) std::snprintf(date, sizeof(date), "%02d/%02d", frame.month, frame.day);
  drawCenteredText(g, 178, 136, date, kActive);

  g.fillRect(74, 38, 34, 13, kDivider);
  g.fillRect(132, 38, 34, 13, kDivider);
  char number[4] = "---";
  if (frame.timeValid) std::snprintf(number, sizeof(number), "%03d", frame.dayOfYear);
  drawSegmentString(g, 77, 38, 8, 13, 1, number);
  char week[3] = "--";
  if (frame.timeValid) std::snprintf(week, sizeof(week), "%02d", frame.isoWeek);
  drawSegmentString(g, 140, 38, 8, 13, 1, week);

  g.fillRect(95, 180, 50, 11, kMint);
  char year[5] = "----";
  if (frame.timeValid) std::snprintf(year, sizeof(year), "%04d", frame.year);
  drawSegmentString(g, 105, 180, 6, 11, 1, year);
}

void FaceRenderer::drawWeatherIcon(Graphics& g, int cx, int cy,
                                   WeatherKind kind, bool isDay, bool stale) {
  using namespace palette;
  const uint16_t c = stale ? kGrey : kActive;
  if (kind == WeatherKind::kClear) {
    if (isDay) {
      g.drawCircle(cx, cy, 7, c);
      g.fillCircle(cx, cy, 5, c);
      for (int i = 0; i < 8; ++i) {
        const float a = i * kPi / 4.0f;
        g.drawLine(cx + static_cast<int>(std::cos(a) * 10), cy + static_cast<int>(std::sin(a) * 10),
                   cx + static_cast<int>(std::cos(a) * 13), cy + static_cast<int>(std::sin(a) * 13), c);
      }
    } else {
      g.fillCircle(cx, cy, 9, c);
      g.fillCircle(cx + 4, cy - 3, 8, kMint);
    }
    return;
  }
  if (kind == WeatherKind::kThunder) {
    g.fillCircle(cx - 5, cy - 3, 6, c); g.fillCircle(cx + 3, cy - 5, 8, c);
    g.fillRect(cx - 9, cy - 3, 18, 7, c);
    g.fillTriangle(cx, cy + 2, cx - 4, cy + 12, cx + 1, cy + 9, c);
    g.fillTriangle(cx + 1, cy + 7, cx + 5, cy + 7, cx - 1, cy + 16, c);
    return;
  }
  if (kind == WeatherKind::kUnknown) {
    drawCenteredText(g, cx, cy - 3, "?", c);
    return;
  }
  // Cloud base, shared by cloudy/rain/snow/fog and partly cloudy.
  if (kind == WeatherKind::kPartlyCloudy) g.fillCircle(cx - 8, cy - 7, 6, c);
  g.fillCircle(cx - 6, cy - 2, 7, c); g.fillCircle(cx + 2, cy - 5, 9, c);
  g.fillCircle(cx + 9, cy, 6, c); g.fillRect(cx - 11, cy, 22, 5, c);
  if (kind == WeatherKind::kRain) {
    g.drawLine(cx - 7, cy + 8, cx - 9, cy + 12, c);
    g.drawLine(cx, cy + 8, cx - 2, cy + 12, c);
    g.drawLine(cx + 7, cy + 8, cx + 5, cy + 12, c);
  } else if (kind == WeatherKind::kSnow) {
    g.drawPixel(cx - 7, cy + 10, c); g.drawPixel(cx, cy + 12, c); g.drawPixel(cx + 7, cy + 10, c);
  } else if (kind == WeatherKind::kFog) {
    g.drawFastHLine(cx - 12, cy + 9, 24, c); g.drawFastHLine(cx - 9, cy + 13, 18, c);
  }
}

void FaceRenderer::drawWeather(Graphics& g, const WeatherData& weather,
                               const ClockFrame& frame, bool fahrenheit) const {
  using namespace palette;
  // The static asset already contains the crisp thermometer, gauge, sun,
  // triangles, labels, and panel shading. Clear only the changing readouts.
  g.fillRect(64, 55, 27, 17, kMint);
  g.fillRect(150, 55, 34, 17, kMint);
  g.fillRect(51, 78, 38, 15, kMint);
  g.fillRect(153, 78, 38, 15, kMint);
  g.fillRect(44, 97, 44, 31, kMint);
  g.fillRect(154, 109, 40, 18, kMint);

  char temperature[8];
  core::formatTemperature(weather.temperature, weather.valid, fahrenheit,
                          temperature, sizeof(temperature));
  const size_t temperatureLength = std::strlen(temperature);
  const char unit = temperatureLength ? temperature[temperatureLength - 1] : (fahrenheit ? 'F' : 'C');
  if (temperatureLength) temperature[temperatureLength - 1] = '\0';
  const int tempX = 86 - static_cast<int>(std::strlen(temperature)) * 7;
  drawSegmentString(g, tempX, 57, 6, 13, 1, temperature);
  char unitText[2] = {unit, '\0'};
  drawText(g, 86, 60, unitText, kActive);
  if (weather.valid) {
    char humidityDigits[4];
    std::snprintf(humidityDigits, sizeof(humidityDigits), "%u", weather.humidity);
    drawSegmentString(g, 151, 57, 6, 13, 1, humidityDigits);
    drawText(g, 176, 60, "%", kActive);
  } else {
    drawSegmentString(g, 158, 57, 6, 13, 1, "--");
  }

  drawSegmentString(g, 51, 79, 6, 13, 1, weather.valid ? weather.sunrise : "--:--");
  drawSegmentString(g, 153, 79, 6, 13, 1, weather.valid ? weather.sunset : "--:--");
  drawWeatherIcon(g, 66, 112, weather.kind, weather.isDay, frame.weatherStale);
  if (frame.weatherStale) drawText(g, 80, 119, "!", kActive);

  char utc[6] = "--:--";
  if (frame.timeValid) std::snprintf(utc, sizeof(utc), "%02d:%02d", frame.utcHour, frame.utcMinute);
  drawSegmentString(g, 155, 111, 7, 14, 1, utc);
}

void FaceRenderer::drawAllDynamic(Graphics& g, const ClockFrame& frame,
                                  const WeatherData& weather,
                                  bool fahrenheit) const {
  drawDial(g, frame);
  drawTimePanel(g, frame);
  drawCalendar(g, frame);
  drawWeather(g, weather, frame, fahrenheit);
}

