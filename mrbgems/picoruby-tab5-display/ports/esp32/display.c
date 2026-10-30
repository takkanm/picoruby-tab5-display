/*
 * The ESP32 implementation of picoruby-tab5-display.
 *
 * This file is compiled by ESP-IDF, not by picoruby's rake build, so it can
 * use ESP-IDF headers directly.
 */
#include "../../include/display.h"

#include "esp_log.h"
#include "tab5_display.h"

static const char *TAG = "picoruby-display";

static bool
ok(const char *what, esp_err_t err)
{
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "%s: %s", what, esp_err_to_name(err));
    return false;
  }
  return true;
}

int
Display_width(void)
{
  return tab5_display_width();
}

int
Display_height(void)
{
  return tab5_display_height();
}

bool
Display_set_rotation(int degrees)
{
  return ok("rotation", tab5_display_set_rotation(degrees));
}

int
Display_rotation(void)
{
  return tab5_display_rotation();
}

bool
Display_fill(uint16_t rgb565)
{
  return ok("fill", tab5_display_fill(rgb565));
}

bool
Display_draw(int x, int y, int w, int h, const uint8_t *pixels)
{
  return ok("draw", tab5_display_draw(x, y, w, h, (const uint16_t *)pixels));
}

bool
Display_set_brightness(int percent)
{
  return ok("brightness", tab5_display_set_brightness(percent));
}
