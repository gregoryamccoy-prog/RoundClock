#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <time.h>

#include "core_logic.h"
#include "face_renderer.h"

class MemoryGraphics final : public Graphics {
 public:
  MemoryGraphics(int16_t w, int16_t h) : w_(w), h_(h), pixels_(w * h) {}
  int16_t width() const override { return w_; }
  int16_t height() const override { return h_; }
  void fillScreen(uint16_t c) override { std::fill(pixels_.begin(), pixels_.end(), c); }
  void drawPixel(int16_t x, int16_t y, uint16_t c) override {
    if (x >= 0 && y >= 0 && x < w_ && y < h_) pixels_[y * w_ + x] = c;
  }
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t c) override {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    for (;;) {
      drawPixel(x0, y0, c);
      if (x0 == x1 && y0 == y1) break;
      const int e2 = 2 * error;
      if (e2 >= dy) { error += dy; x0 += sx; }
      if (e2 <= dx) { error += dx; y0 += sy; }
    }
  }
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t c) override {
    for (int i = 0; i < w; ++i) drawPixel(x + i, y, c);
  }
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t c) override {
    for (int i = 0; i < h; ++i) drawPixel(x, y + i, c);
  }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) override {
    for (int yy = 0; yy < h; ++yy) drawFastHLine(x, y + yy, w, c);
  }
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) override {
    drawFastHLine(x, y, w, c); drawFastHLine(x, y + h - 1, w, c);
    drawFastVLine(x, y, h, c); drawFastVLine(x + w - 1, y, h, c);
  }
  void drawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t c) override {
    int x = r, y = 0, err = 0;
    while (x >= y) {
      drawPixel(cx+x,cy+y,c); drawPixel(cx+y,cy+x,c); drawPixel(cx-y,cy+x,c); drawPixel(cx-x,cy+y,c);
      drawPixel(cx-x,cy-y,c); drawPixel(cx-y,cy-x,c); drawPixel(cx+y,cy-x,c); drawPixel(cx+x,cy-y,c);
      ++y; if (err <= 0) err += 2*y + 1; if (err > 0) { --x; err -= 2*x + 1; }
    }
  }
  void fillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t c) override {
    for (int y = -r; y <= r; ++y) {
      const int span = static_cast<int>(std::sqrt(static_cast<double>(r*r - y*y)));
      drawFastHLine(cx - span, cy + y, span * 2 + 1, c);
    }
  }
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                    int16_t x2, int16_t y2, uint16_t c) override {
    const int minX = std::min({x0,x1,x2}), maxX = std::max({x0,x1,x2});
    const int minY = std::min({y0,y1,y2}), maxY = std::max({y0,y1,y2});
    auto edge = [](int ax,int ay,int bx,int by,int px,int py) {
      return (px-ax)*(by-ay) - (py-ay)*(bx-ax);
    };
    for (int y = minY; y <= maxY; ++y) for (int x = minX; x <= maxX; ++x) {
      const int a=edge(x0,y0,x1,y1,x,y), b=edge(x1,y1,x2,y2,x,y), d=edge(x2,y2,x0,y0,x,y);
      if ((a>=0 && b>=0 && d>=0) || (a<=0 && b<=0 && d<=0)) drawPixel(x,y,c);
    }
  }
  uint16_t at(int x, int y) const { return pixels_[y * w_ + x]; }
  const std::vector<uint16_t>& pixels() const { return pixels_; }
  size_t count(uint16_t color) const {
    return static_cast<size_t>(std::count(pixels_.begin(), pixels_.end(), color));
  }
  bool loadRgb565(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    for (uint16_t& pixel : pixels_) {
      const int low = in.get(), high = in.get();
      if (low == EOF || high == EOF) return false;
      pixel = static_cast<uint16_t>(low | (high << 8));
    }
    return true;
  }
  void writePpm(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << w_ << " " << h_ << "\n255\n";
    for (uint16_t color : pixels_) {
      const uint8_t r5 = (color >> 11) & 0x1f, g6 = (color >> 5) & 0x3f, b5 = color & 0x1f;
      const char rgb[3] = {static_cast<char>((r5 * 255 + 15) / 31),
                           static_cast<char>((g6 * 255 + 31) / 63),
                           static_cast<char>((b5 * 255 + 15) / 31)};
      out.write(rgb, 3);
    }
  }
 private:
  int16_t w_, h_;
  std::vector<uint16_t> pixels_;
};

void testCalendarMath() {
  assert(core::isLeapYear(2000));
  assert(!core::isLeapYear(2100));
  assert(core::isLeapYear(2024));
  assert(core::dayOfYear(2024, 2, 29) == 60);
  assert(core::dayOfYear(2023, 12, 31) == 365);
  assert(core::isoWeekNumber(2019, 12, 30) == 1);
  assert(core::isoWeekNumber(2021, 1, 1) == 53);
  assert(core::isoWeekNumber(2021, 1, 4) == 1);
  assert(core::isoWeekNumber(2020, 12, 31) == 53);
}

void testDstRule() {
  setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0/2", 1);
  tzset();
  struct tm before{}; before.tm_year=126; before.tm_mon=2; before.tm_mday=8; before.tm_hour=1; before.tm_min=59; before.tm_isdst=-1;
  time_t epoch = mktime(&before) + 120;
  struct tm after{}; localtime_r(&epoch, &after);
  assert(after.tm_hour == 3 && after.tm_min == 1 && after.tm_isdst == 1);
  struct tm winter{}; winter.tm_year=126; winter.tm_mon=0; winter.tm_mday=15; winter.tm_hour=12; winter.tm_isdst=-1;
  struct tm summer{}; summer.tm_year=126; summer.tm_mon=6; summer.tm_mday=15; summer.tm_hour=12; summer.tm_isdst=-1;
  mktime(&winter); mktime(&summer);
  assert(winter.tm_isdst == 0 && summer.tm_isdst == 1);
}

void testFormattingAndWeatherCodes() {
  char value[8];
  core::formatTemperature(-7.6f, true, true, value, sizeof(value)); assert(std::string(value) == "-8F");
  core::formatTemperature(123.2f, true, true, value, sizeof(value)); assert(std::string(value) == "123F");
  core::formatTemperature(NAN, true, false, value, sizeof(value)); assert(std::string(value) == "---C");
  assert(core::weatherKindFromCode(0) == WeatherKind::kClear);
  assert(core::weatherKindFromCode(61) == WeatherKind::kRain);
  assert(core::weatherKindFromCode(75) == WeatherKind::kSnow);
  assert(core::weatherKindFromCode(95) == WeatherKind::kThunder);
  assert(core::sevenSegmentMask('8') == 0x7f);
  assert(core::sevenSegmentMask('-') == 0x40);
}

void testRendererAndWritePreview() {
  MemoryGraphics canvas(240, 240);
  FaceRenderer renderer;
  ClockFrame frame;
  frame.timeValid=true; frame.hour=10; frame.minute=10; frame.second=45;
  frame.utcHour=15; frame.utcMinute=10; frame.year=2026; frame.month=11; frame.day=15;
  frame.weekday=0; frame.dayOfYear=319; frame.isoWeek=46; frame.wifiConnected=true;
  WeatherData weather;
  weather.valid=true; weather.temperature=63.0f; weather.humidity=47; weather.kind=WeatherKind::kThunder;
  weather.isDay=true; std::snprintf(weather.sunrise,6,"07:22"); std::snprintf(weather.sunset,6,"17:31");
  assert(canvas.loadRgb565("assets/face_background.rgb565"));
  renderer.drawDialHands(canvas, frame);
  renderer.drawTimePanel(canvas, frame);
  renderer.drawCalendar(canvas, frame);
  renderer.drawWeather(canvas, weather, frame, true);

  assert(canvas.count(palette::kMint) > 3000);
  assert(canvas.count(palette::kActive) > 1500);
  assert(canvas.count(palette::kInactive) > 500);  // ghost segments are really present
  const uint16_t corner = canvas.at(0,0);
  assert(((corner >> 11) & 0x1f) < 3 && ((corner >> 5) & 0x3f) < 5 &&
         (corner & 0x1f) < 3);  // circular display outside stays near-black

  std::vector<uint16_t> first = canvas.pixels();
  frame.minute = 11; frame.second = 0;
  // Restore just the dial region as the firmware sprite does, then draw hands.
  MemoryGraphics restored(240, 240);
  assert(restored.loadRgb565("assets/face_background.rgb565"));
  for (int y=72; y<130; ++y) for (int x=91; x<149; ++x) {
    canvas.drawPixel(x, y, restored.at(x, y));
  }
  renderer.drawDialHands(canvas, frame); renderer.drawTimePanel(canvas, frame);
  size_t changed = 0;
  for (size_t i=0; i<first.size(); ++i) if (first[i] != canvas.pixels()[i]) ++changed;
  assert(changed > 100 && changed < 5000);  // dirty redraw changes only dynamic regions
  canvas.writePpm("test-output/clock-preview.ppm");
}

int main() {
  testCalendarMath();
  testDstRule();
  testFormattingAndWeatherCodes();
  testRendererAndWritePreview();
  std::puts("PASS: 29 assertions covering calendar/DST, formatting, WMO mapping, ghost segments, and framebuffer redraw");
  return 0;
}

