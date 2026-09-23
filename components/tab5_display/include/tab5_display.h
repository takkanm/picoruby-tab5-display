/*
 * The MIPI-DSI panel of the M5Stack Tab5.
 *
 * The real work is done by the Tab5 BSP from esp-bsp
 * (espressif/m5stack_tab5): I2C, releasing the LCD and touch resets through
 * the GPIO expanders, DSI PHY power, and detecting the panel generation
 * (ILI9881C / ST7123 / ST7121) at runtime. This is a thin wrapper around it
 * plus the drawing entry points.
 *
 * The panel is portrait: 720 (W) x 1280 (H), RGB565.
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TAB5_DISPLAY_WIDTH   720
#define TAB5_DISPLAY_HEIGHT  1280

/*
 * Initialise the panel, turn the display on and light the backlight.
 * Call this before picoruby_esp32(). Calling it again is a no-op that
 * returns ESP_OK.
 */
esp_err_t tab5_display_init(void);

/* Fill the whole screen with one RGB565 colour. */
esp_err_t tab5_display_fill(uint16_t rgb565);

/*
 * Draw an RGB565 bitmap of w x h pixels at (x, y).
 * `pixels` holds w * h contiguous elements (stride = w).
 */
esp_err_t tab5_display_draw(int x, int y, int w, int h, const uint16_t *pixels);

/* Backlight brightness, 0-100 [%]. */
esp_err_t tab5_display_set_brightness(int percent);

#ifdef __cplusplus
}
#endif
