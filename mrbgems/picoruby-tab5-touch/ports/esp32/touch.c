/*
 * The ESP32 implementation of picoruby-tab5-touch.
 *
 * Compiled by ESP-IDF as part of components/tab5_display, like the display
 * port next to it.
 */
#include "../../include/touch.h"

#include "esp_log.h"
#include "tab5_touch.h"

static const char *TAG = "picoruby-touch";

bool
Touch_point(bool *touched, int *x, int *y)
{
  esp_err_t err = tab5_touch_point(touched, x, y);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "point: %s", esp_err_to_name(err));
    return false;
  }
  return true;
}
