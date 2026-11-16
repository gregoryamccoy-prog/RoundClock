#pragma once

#include <stddef.h>
#include <stdint.h>

#include "weather_data.h"

namespace core {

uint8_t sevenSegmentMask(char c);
bool isLeapYear(int year);
int dayOfYear(int year, int month, int day);
int isoWeekNumber(int year, int month, int day);
WeatherKind weatherKindFromCode(int code);
void formatTemperature(float value, bool valid, bool fahrenheit,
                       char* output, size_t outputSize);
void formatTwoDigits(int value, char output[3]);

}  // namespace core

