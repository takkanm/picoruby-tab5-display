#ifndef PICORUBY_TAB5_GPIO_DRIVE_H_
#define PICORUBY_TAB5_GPIO_DRIVE_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The VM-independent entry point, implemented in ports/<platform>/gpio_drive.c.
 * strength is 0 (weakest) to 3 (strongest). Returns false on an invalid pin.
 */
bool GPIODrive_set(int pin, int strength);

#ifdef __cplusplus
}
#endif

#endif /* PICORUBY_TAB5_GPIO_DRIVE_H_ */
