#include "tab5_touch.h"

#include "esp_check.h"
#include "esp_lcd_touch.h"

#include "bsp/esp-bsp.h"
#include "bsp/touch.h"

static const char *TAG = "tab5_touch";

static esp_lcd_touch_handle_t s_touch = NULL;

esp_err_t
tab5_touch_point(bool *touched, int *x, int *y)
{
  if (s_touch == NULL) {
    ESP_RETURN_ON_ERROR(bsp_touch_new(NULL, &s_touch), TAG, "init");
    ESP_LOGI(TAG, "touch ready");
  }

  ESP_RETURN_ON_ERROR(esp_lcd_touch_read_data(s_touch), TAG, "read");

  esp_lcd_touch_point_data_t point;
  uint8_t count = 0;
  ESP_RETURN_ON_ERROR(esp_lcd_touch_get_data(s_touch, &point, &count, 1), TAG, "get data");

  *touched = count > 0;
  if (*touched) {
    *x = point.x;
    *y = point.y;
  }
  return ESP_OK;
}
