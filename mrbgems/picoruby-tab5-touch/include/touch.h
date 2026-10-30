#ifndef PICORUBY_TAB5_TOUCH_H_
#define PICORUBY_TAB5_TOUCH_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The VM-independent entry point, implemented in ports/<platform>/touch.c.
 *
 * Reads the panel once. Sets *touched, and when it is true the first
 * touch point in physical panel coordinates (0..719, 0..1279), which do not
 * follow Display.rotation. Returns false when the panel could not be read.
 */
bool Touch_point(bool *touched, int *x, int *y);

#ifdef __cplusplus
}
#endif

#endif /* PICORUBY_TAB5_TOUCH_H_ */
