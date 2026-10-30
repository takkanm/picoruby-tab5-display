/*
 * The touch panel of the M5Stack Tab5.
 *
 * The controller differs by board generation (GT911, or ST7123 for the
 * ST7123 / ST7121 panels); the BSP's bsp_touch_new() picks the right one.
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Read the panel once. *touched tells whether anything touches it; if so,
 * (*x, *y) is the first touch point in physical panel coordinates. The
 * panel is initialised on the first call.
 */
esp_err_t tab5_touch_point(bool *touched, int *x, int *y);

#ifdef __cplusplus
}
#endif
