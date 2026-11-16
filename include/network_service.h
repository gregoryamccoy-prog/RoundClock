#pragma once

#include <Arduino.h>
#include <atomic>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct NetworkState {
  std::atomic<bool> wifiConnected{false};
  std::atomic<bool> timeSynchronized{false};
  std::atomic<uint32_t> lastNtpSyncMs{0};
  std::atomic<uint32_t> weatherRevision{0};
};

class NetworkService {
 public:
  NetworkService(QueueHandle_t weatherQueue, NetworkState& state, bool fahrenheit)
      : weatherQueue_(weatherQueue), state_(state), fahrenheit_(fahrenheit) {}
  void begin();

 private:
  static void taskEntry(void* context);
  void taskLoop();
  bool fetchWeather();

  QueueHandle_t weatherQueue_;
  NetworkState& state_;
  bool fahrenheit_;
};

