#pragma once

#include <stdint.h>

class Graphics {
 public:
  virtual ~Graphics() = default;
  virtual int16_t width() const = 0;
  virtual int16_t height() const = 0;
  virtual void fillScreen(uint16_t color) = 0;
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
  virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                        uint16_t color) = 0;
  virtual void drawFastHLine(int16_t x, int16_t y, int16_t w,
                             uint16_t color) = 0;
  virtual void drawFastVLine(int16_t x, int16_t y, int16_t h,
                             uint16_t color) = 0;
  virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                        uint16_t color) = 0;
  virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                        uint16_t color) = 0;
  virtual void drawCircle(int16_t x, int16_t y, int16_t r,
                          uint16_t color) = 0;
  virtual void fillCircle(int16_t x, int16_t y, int16_t r,
                          uint16_t color) = 0;
  virtual void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            int16_t x2, int16_t y2, uint16_t color) = 0;
};

