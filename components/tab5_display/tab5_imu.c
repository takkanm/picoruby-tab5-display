#include "tab5_imu.h"

#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bmi270.h"
#include "bsp/esp-bsp.h"

static const char *TAG = "tab5_imu";

static bmi270_handle_t *s_bmi270 = NULL;

/*
 * bmi270_create() soft-resets the chip and then waits pdMS_TO_TICKS(10)
 * before talking to it again. With CONFIG_FREERTOS_HZ=100 that is a single
 * tick, i.e. anything from 0 to 10 ms, and the first read after the reset
 * sometimes fails with ESP_ERR_INVALID_STATE. Trying again after a proper
 * pause is enough.
 */
#define CREATE_ATTEMPTS 3
#define RETRY_DELAY_MS  50

static esp_err_t
init(void)
{
  ESP_RETURN_ON_ERROR(bsp_i2c_init(), TAG, "i2c");

  const bmi270_driver_config_t driver = {
    .addr = BMI270_I2C_ADDRESS_L,
    .interface = BMI270_USE_I2C,
    .i2c_bus = bsp_i2c_get_handle(),
  };
  esp_err_t err = ESP_FAIL;
  for (int i = 0; i < CREATE_ATTEMPTS && err != ESP_OK; i++) {
    if (i > 0) {
      ESP_LOGW(TAG, "BMI270 init failed (%s), retrying", esp_err_to_name(err));
      vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
    }
    err = bmi270_create(&driver, &s_bmi270);
  }
  ESP_RETURN_ON_ERROR(err, TAG, "create");

  /*
   * Orientation only needs gravity, so a low rate and the narrowest range
   * (best resolution) do. The gyroscope has to be given a setting too.
   */
  const bmi270_config_t config = {
    .acce_odr = BMI270_ACC_ODR_25_HZ,
    .acce_range = BMI270_ACC_RANGE_2_G,
    .gyro_odr = BMI270_GYR_ODR_25_HZ,
    .gyro_range = BMI270_GYR_RANGE_2000_DPS,
  };
  err = bmi270_start(s_bmi270, &config);
  if (err != ESP_OK) {
    bmi270_delete(s_bmi270);
    s_bmi270 = NULL;
    ESP_RETURN_ON_ERROR(err, TAG, "start");
  }
  ESP_LOGI(TAG, "BMI270 ready");
  return ESP_OK;
}

esp_err_t
tab5_imu_acceleration(float *x, float *y, float *z)
{
  if (s_bmi270 == NULL) {
    ESP_RETURN_ON_ERROR(init(), TAG, "init");
  }
  return bmi270_get_acce_data(s_bmi270, x, y, z);
}
