#include "display_service.h"

#include "board_config.h"
#include "tft_graphics.h"

extern const uint8_t faceBackgroundStart[]
    asm("_binary_assets_face_background_rgb565_start");

namespace {
const uint16_t* faceBackground() {
  return reinterpret_cast<const uint16_t*>(faceBackgroundStart);
}

void restoreBackgroundRegion(TFT_eSprite& sprite, int sourceX, int sourceY,
                             int width, int height) {
  const uint16_t* background = faceBackground();
  sprite.setSwapBytes(false);
  for (int row = 0; row < height; ++row) {
    sprite.pushImage(0, row, width, 1,
                     background + (sourceY + row) * config::kScreenWidth + sourceX);
  }
}
}  // namespace

DisplayService::DisplayService()
    : tft_(), dialSprite_(&tft_), timeSprite_(&tft_) {}

void DisplayService::begin(const ClockFrame& frame, const WeatherData& weather,
                           bool fahrenheit) {
  tft_.init();
  tft_.setRotation(config::kDisplayRotation);
  tft_.invertDisplay(true);
  tft_.setSwapBytes(true);
  tft_.pushImage(0, 0, config::kScreenWidth, config::kScreenHeight,
                 faceBackground());

  dialSprite_.setColorDepth(16);
  timeSprite_.setColorDepth(16);
  dialSprite_.createSprite(58, 58);
  timeSprite_.createSprite(142, 30);

  TftGraphics screen(tft_);
  renderer_.drawDialHands(screen, frame);
  renderer_.drawTimePanel(screen, frame);
  renderer_.drawCalendar(screen, frame);
  renderer_.drawWeather(screen, weather, frame, fahrenheit);
}

void DisplayService::update(const ClockFrame& frame, const WeatherData& weather,
                            bool fahrenheit, bool calendarChanged,
                            bool weatherChanged) {
  {
    restoreBackgroundRegion(dialSprite_, 91, 72, 58, 58);
    TftGraphics dial(dialSprite_, 91, 72, 58, 58);
    renderer_.drawDialHands(dial, frame);
    dialSprite_.pushSprite(91, 72);
  }
  {
    timeSprite_.fillSprite(palette::kMint);
    TftGraphics time(timeSprite_, 49, 148, 142, 30);
    renderer_.drawTimePanel(time, frame);
    timeSprite_.pushSprite(49, 148);
  }
  TftGraphics screen(tft_);
  if (calendarChanged) renderer_.drawCalendar(screen, frame);
  // UTC changes each minute, so this is intentionally true on minute changes
  // even when the weather snapshot itself did not change.
  if (weatherChanged) renderer_.drawWeather(screen, weather, frame, fahrenheit);
}

