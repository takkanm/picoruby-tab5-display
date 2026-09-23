#ifndef PICORUBY_TAB5_DISPLAY_H_
#define PICORUBY_TAB5_DISPLAY_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The VM-independent entry points. They are implemented in
 * ports/<platform>/display.c.
 *
 * The ports flatten platform-specific error types into bool so that this
 * layer stays free of, say, ESP-IDF types.
 */

int  Display_width(void);
int  Display_height(void);

/* Fill the whole screen with one RGB565 colour. */
bool Display_fill(uint16_t rgb565);

/*
 * Draw an RGB565 bitmap of w x h pixels at (x, y).
 * `pixels` is w * h * 2 contiguous bytes, little-endian, row-major.
 * The caller (the VM binding) must have validated the length already.
 */
bool Display_draw(int x, int y, int w, int h, const uint8_t *pixels);

/* Backlight brightness, 0-100 [%]. */
bool Display_set_brightness(int percent);

#ifdef __cplusplus
}
#endif

#endif /* PICORUBY_TAB5_DISPLAY_H_ */
