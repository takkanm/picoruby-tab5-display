/*
 * The ESP32 implementation of picoruby-tab5-imu.
 *
 * Compiled by ESP-IDF as part of components/tab5_display, like the display
 * port next to it.
 */
#include "../../include/imu.h"

#include "esp_log.h"
#include "tab5_imu.h"

static const char *TAG = "picoruby-imu";

bool
IMU_acceleration(float *x, float *y, float *z)
{
  esp_err_t err = tab5_imu_acceleration(x, y, z);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "acceleration: %s", esp_err_to_name(err));
    return false;
  }
  return true;
}
