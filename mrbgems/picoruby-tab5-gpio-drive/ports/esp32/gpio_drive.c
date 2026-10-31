/*
 * The ESP32 implementation of picoruby-tab5-gpio-drive.
 *
 * Compiled by ESP-IDF as part of components/tab5_display, like the display
 * port next to it. Nothing here is specific to the Tab5.
 */
#include "../../include/gpio_drive.h"

#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "picoruby-gpio-drive";

bool
GPIODrive_set(int pin, int strength)
{
  if (!GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    return false;
  }
  esp_err_t err = gpio_set_drive_capability(pin, (gpio_drive_cap_t)strength);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "set: %s", esp_err_to_name(err));
    return false;
  }
  return true;
}
