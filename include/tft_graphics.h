#pragma once

#include <TFT_eSPI.h>

#include "graphics.h"

class TftGraphics final : public Graphics {
 public:
  TftGraphics(TFT_eSPI& target, int16_t originX = 0, int16_t originY = 0,
              int16_t width = 240, int16_t height = 240)
      : target_(target), originX_(originX), originY_(originY),
        width_(width), height_(height) {}

  int16_t width() const override { return width_; }
  int16_t height() const override { return height_; }
  void fillScreen(uint16_t c) override { target_.fillScreen(c); }
  void drawPixel(int16_t x, int16_t y, uint16_t c) override {
    target_.drawPixel(x - originX_, y - originY_, c);
  }
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                uint16_t c) override {
    target_.drawLine(x0 - originX_, y0 - originY_, x1 - originX_, y1 - originY_, c);
  }
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t c) override {
    target_.drawFastHLine(x - originX_, y - originY_, w, c);
  }
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t c) override {
    target_.drawFastVLine(x - originX_, y - originY_, h, c);
  }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                uint16_t c) override {
    target_.fillRect(x - originX_, y - originY_, w, h, c);
  }
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                uint16_t c) override {
    target_.drawRect(x - originX_, y - originY_, w, h, c);
  }
  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t c) override {
    target_.drawCircle(x - originX_, y - originY_, r, c);
  }
  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t c) override {
    target_.fillCircle(x - originX_, y - originY_, r, c);
  }
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                    int16_t x2, int16_t y2, uint16_t c) override {
    target_.fillTriangle(x0 - originX_, y0 - originY_, x1 - originX_,
                         y1 - originY_, x2 - originX_, y2 - originY_, c);
  }

 private:
  TFT_eSPI& target_;
  int16_t originX_;
  int16_t originY_;
  int16_t width_;
  int16_t height_;
};

