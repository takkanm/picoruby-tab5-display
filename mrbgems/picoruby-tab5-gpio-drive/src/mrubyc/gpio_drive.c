#include <mrubyc.h>
#include "../../include/gpio_drive.h"

/*
 * GPIODrive.set(pin, strength)
 *
 * strength is 0 (weakest) to 3 (strongest); the ESP32 default is 2.
 * Call it after GPIO.new: picoruby's ESP32 GPIO port resets the pin
 * (gpio_reset_pin), which puts the drive strength back to the default.
 */
static void
c_GPIODrive_set(mrbc_vm *vm, mrbc_value *v, int argc)
{
  if (argc != 2) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments. expected 2");
    return;
  }
  if (v[1].tt != MRBC_TT_INTEGER || v[2].tt != MRBC_TT_INTEGER) {
    mrbc_raise(vm, MRBC_CLASS(TypeError), "pin and strength must be Integer");
    return;
  }
  int strength = GET_INT_ARG(2);
  if (strength < 0 || 3 < strength) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "strength must be within 0..3");
    return;
  }
  if (!GPIODrive_set(GET_INT_ARG(1), strength)) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "invalid pin");
    return;
  }
  SET_INT_RETURN(strength);
}

/* See src/mrubyc/display.c in picoruby-tab5-display for how this name is derived. */
void
mrbc_tab5_gpio_drive_init(mrbc_vm *vm)
{
  mrbc_class *mrbc_class_GPIODrive = mrbc_define_class(vm, "GPIODrive", mrbc_class_object);

  mrbc_define_method(vm, mrbc_class_GPIODrive, "set", c_GPIODrive_set);
}
