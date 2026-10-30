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
 *
 * Drawing can be rotated in steps of 90 degrees (tab5_display_set_rotation).
 * Coordinates passed to tab5_display_draw() are then in the rotated, logical
 * frame, whose size tab5_display_width() / tab5_display_height() report.
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The physical panel size, independent of the rotation. */
#define TAB5_DISPLAY_WIDTH   720
#define TAB5_DISPLAY_HEIGHT  1280

/*
 * Initialise the panel, turn the display on and light the backlight.
 * Call this before picoruby_esp32(). Calling it again is a no-op that
 * returns ESP_OK.
 */
esp_err_t tab5_display_init(void);

/*
 * Rotate everything drawn from now on by `degrees` clockwise: 0, 90, 180 or
 * 270. With 90 the logical top-left corner is the panel's top-right corner,
 * so a portrait panel turned on its left side shows an upright landscape
 * picture. What is already on screen is left as it is.
 */
esp_err_t tab5_display_set_rotation(int degrees);
int tab5_display_rotation(void);

/* The logical screen size for the current rotation. */
int tab5_display_width(void);
int tab5_display_height(void);

/* Fill the whole screen with one RGB565 colour. */
esp_err_t tab5_display_fill(uint16_t rgb565);

/*
 * Draw an RGB565 bitmap of w x h pixels at (x, y) in logical coordinates.
 * `pixels` holds w * h contiguous elements (stride = w).
 */
esp_err_t tab5_display_draw(int x, int y, int w, int h, const uint16_t *pixels);

/* Backlight brightness, 0-100 [%]. */
esp_err_t tab5_display_set_brightness(int percent);

#ifdef __cplusplus
}
#endif
