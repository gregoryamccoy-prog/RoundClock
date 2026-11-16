#include "core_logic.h"

#include <cmath>
#include <cstdio>

namespace core {
namespace {

int dayOfWeekMonday1(int year, int month, int day) {
  // Sakamoto's algorithm: 0=Sunday; convert to ISO 1=Monday..7=Sunday.
  static const int offsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (month < 3) --year;
  const int sunday0 =
      (year + year / 4 - year / 100 + year / 400 + offsets[month - 1] + day) % 7;
  return sunday0 == 0 ? 7 : sunday0;
}

int isoWeeksInYear(int year) {
  const int jan1 = dayOfWeekMonday1(year, 1, 1);
  return (jan1 == 4 || (jan1 == 3 && isLeapYear(year))) ? 53 : 52;
}

}  // namespace

uint8_t sevenSegmentMask(char c) {
  // Bits are A B C D E F G from bit 0 through bit 6.
  static constexpr uint8_t masks[] = {
      0x3F, 0x06, 0x5B, 0x4F, 0x66,
      0x6D, 0x7D, 0x07, 0x7F, 0x6F};
  if (c >= '0' && c <= '9') return masks[c - '0'];
  if (c == '-') return 0x40;
  return 0;
}

bool isLeapYear(int year) {
  return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int dayOfYear(int year, int month, int day) {
  static constexpr int beforeMonth[] =
      {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  if (month < 1 || month > 12 || day < 1 || day > 31) return 0;
  int result = beforeMonth[month] + day;
  if (month > 2 && isLeapYear(year)) ++result;
  return result;
}

int isoWeekNumber(int year, int month, int day) {
  const int doy = dayOfYear(year, month, day);
  if (doy == 0) return 0;
  const int weekday = dayOfWeekMonday1(year, month, day);
  int week = (doy - weekday + 10) / 7;
  if (week < 1) return isoWeeksInYear(year - 1);
  if (week > isoWeeksInYear(year)) return 1;
  return week;
}

WeatherKind weatherKindFromCode(int code) {
  if (code == 0) return WeatherKind::kClear;
  if (code >= 1 && code <= 2) return WeatherKind::kPartlyCloudy;
  if (code == 3) return WeatherKind::kCloudy;
  if (code == 45 || code == 48) return WeatherKind::kFog;
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) {
    return WeatherKind::kRain;
  }
  if ((code >= 71 && code <= 77) || (code >= 85 && code <= 86)) {
    return WeatherKind::kSnow;
  }
  if (code >= 95 && code <= 99) return WeatherKind::kThunder;
  return WeatherKind::kUnknown;
}

void formatTemperature(float value, bool valid, bool fahrenheit,
                       char* output, size_t outputSize) {
  if (outputSize == 0) return;
  if (!valid || !std::isfinite(value)) {
    std::snprintf(output, outputSize, "---%c", fahrenheit ? 'F' : 'C');
    return;
  }
  int rounded = static_cast<int>(std::lround(value));
  if (rounded < -99) rounded = -99;
  if (rounded > 199) rounded = 199;
  std::snprintf(output, outputSize, "%d%c", rounded, fahrenheit ? 'F' : 'C');
}

void formatTwoDigits(int value, char output[3]) {
  if (value < 0 || value > 99) {
    output[0] = '-';
    output[1] = '-';
  } else {
    output[0] = static_cast<char>('0' + value / 10);
    output[1] = static_cast<char>('0' + value % 10);
  }
  output[2] = '\0';
}

}  // namespace core

