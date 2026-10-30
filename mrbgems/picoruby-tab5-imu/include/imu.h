#ifndef PICORUBY_TAB5_IMU_H_
#define PICORUBY_TAB5_IMU_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The VM-independent entry point, implemented in ports/<platform>/imu.c.
 *
 * Acceleration in g along the sensor's axes. Returns false when the sensor
 * could not be read.
 *
 * Measured on a Tab5: in each pose below, the listed axis reads about 1 g
 * with that sign.
 *   +y  portrait, USB-C at the bottom
 *   -y  portrait, upside down
 *   -x  landscape, USB-C on the right
 *   +x  landscape, USB-C on the left
 *   -z  lying flat, screen up
 */
bool IMU_acceleration(float *x, float *y, float *z);

#ifdef __cplusplus
}
#endif

#endif /* PICORUBY_TAB5_IMU_H_ */
