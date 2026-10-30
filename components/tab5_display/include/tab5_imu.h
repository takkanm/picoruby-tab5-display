/*
 * The BMI270 accelerometer of the M5Stack Tab5.
 *
 * It sits on the same internal I2C bus as the touch controller and the GPIO
 * expanders, so it uses the BSP's bus handle. The driver is esp-bsp's
 * espressif/bmi270, which the Tab5 BSP already depends on.
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Read the acceleration in g. The sensor is initialised on the first call;
 * unlike the display, nothing depends on when that happens.
 */
esp_err_t tab5_imu_acceleration(float *x, float *y, float *z);

#ifdef __cplusplus
}
#endif
