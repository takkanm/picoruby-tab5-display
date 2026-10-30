#include <mrubyc.h>
#include "../../include/imu.h"

/*
 * IMU.acceleration #=> [x, y, z]
 *
 * Floats in g. The sensor is set up on the first call, which takes a
 * moment: the BMI270 needs its configuration blob uploaded over I2C.
 */
static void
c_IMU_acceleration(mrbc_vm *vm, mrbc_value *v, int argc)
{
  float x, y, z;
  if (!IMU_acceleration(&x, &y, &z)) {
    mrbc_raise(vm, MRBC_CLASS(RuntimeError), "could not read the accelerometer");
    return;
  }

  mrbc_value ary = mrbc_array_new(vm, 3);
  mrbc_value f;
  f = mrbc_float_value(vm, x);
  mrbc_array_push(&ary, &f);
  f = mrbc_float_value(vm, y);
  mrbc_array_push(&ary, &f);
  f = mrbc_float_value(vm, z);
  mrbc_array_push(&ary, &f);
  SET_RETURN(ary);
}

/* See src/mrubyc/display.c in picoruby-tab5-display for how this name is derived. */
void
mrbc_tab5_imu_init(mrbc_vm *vm)
{
  mrbc_class *mrbc_class_IMU = mrbc_define_class(vm, "IMU", mrbc_class_object);

  mrbc_define_method(vm, mrbc_class_IMU, "acceleration", c_IMU_acceleration);
}
