#pragma once

#include <TFT_eSPI.h>

#include "face_renderer.h"

class DisplayService {
 public:
  DisplayService();
  void begin(const ClockFrame& frame, const WeatherData& weather, bool fahrenheit);
  void update(const ClockFrame& frame, const WeatherData& weather, bool fahrenheit,
              bool calendarChanged, bool weatherChanged);

 private:
  TFT_eSPI tft_;
  TFT_eSprite dialSprite_;
  TFT_eSprite timeSprite_;
  FaceRenderer renderer_;
};

