# ESP32 RoundClock

A PlatformIO/Arduino firmware recreation of the supplied `watchFace.png` for a
Waveshare 1.28-inch, 240x240 GC9A01 round LCD and an ESP32-WROOM-32 board.

The face uses a diffusion-assisted, cleaned static background converted to a
115,200-byte RGB565 flash asset. All sample values were removed before export;
time, hands, weather, calendar, UTC, status icons, and ghosted active/inactive
segments remain deterministic C++ drawing. The final composite is exercised by
the host-side framebuffer tests.

## Hardware wiring

| Display | ESP32 | Purpose |
|---|---:|---|
| VCC | 3V3 | display power |
| GND | GND | common ground |
| DIN | GPIO23 | SPI MOSI |
| CLK | GPIO18 | SPI clock |
| CS | GPIO27 | chip select |
| DC | GPIO26 | data/command |
| RST | GPIO25 | reset |
| BL | 3V3 | fixed backlight for the first build |

Do not drive the LED backlight directly from a GPIO. The optional PWM path in
`include/board_config.h` assumes a suitable transistor or a confirmed
logic-level BL input.

## Build, test, and flash

Install the VS Code PlatformIO extension, open this folder, then use PlatformIO's
Build and Upload buttons. The equivalent commands are:

```bash
~/.platformio/penv/bin/pio run
~/.platformio/penv/bin/pio run -t upload
~/.platformio/penv/bin/pio device monitor
```

Run the dependency-free renderer/date/weather tests with:

```bash
./test/run_host_tests.sh
```

They also write `test-output/clock-preview.png`, a full in-memory render of the
native 240x240 face, plus `clock-preview-review.png`, a 4x nearest-neighbor copy
for easier review. The test output is ignored by Git.

## Configuration

- `include/board_config.h`: pins, timezone, coordinates, units, update rates.
- `secrets.h`: ignored local forwarding file that reuses the Wi-Fi credentials
  from `../WeatherStation/secrets.h`. Replace it with local `WIFI_SSID` and
  `WIFI_PASSWORD` defines if this project is moved elsewhere.
- 12/24-hour and brightness settings are stored in NVS only when changed.

The current timezone rule is Eastern US with DST and the coordinates match the
existing WeatherStation project. HTTPS validates Open-Meteo against the ISRG
Root X1 public trust anchor; requests fail closed rather than disabling
certificate checks.

## Firmware structure

- `main.cpp`: application lifecycle and display cadence.
- `display_service.cpp`: sole owner of TFT_eSPI and small anti-flicker sprites.
- `face_renderer.cpp`: hardware-independent visual rendering.
- `assets/face_background.rgb565`: static 240x240 flash-resident face artwork.
- `network_service.cpp`: core-0 Wi-Fi, SNTP, HTTPS weather, retry/backoff.
- `core_logic.cpp`: tested date, ISO week, segment, formatting, WMO mapping.
- `settings.cpp`: low-write NVS settings.

The display loop remains on the Arduino core while HTTPS runs in a separate
FreeRTOS task. Weather data crosses through a one-element overwrite queue.

