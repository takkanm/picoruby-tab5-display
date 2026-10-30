#include <mrubyc.h>
#include "../../include/touch.h"

/*
 * Touch.point #=> [x, y] or nil
 *
 * The first touch point in physical panel coordinates, or nil when nothing
 * touches the panel. The panel is set up on the first call.
 */
static void
c_Touch_point(mrbc_vm *vm, mrbc_value *v, int argc)
{
  bool touched;
  int x, y;
  if (!Touch_point(&touched, &x, &y)) {
    mrbc_raise(vm, MRBC_CLASS(RuntimeError), "could not read the touch panel");
    return;
  }
  if (!touched) {
    SET_NIL_RETURN();
    return;
  }

  mrbc_value ary = mrbc_array_new(vm, 2);
  mrbc_value n;
  n = mrbc_integer_value(x);
  mrbc_array_push(&ary, &n);
  n = mrbc_integer_value(y);
  mrbc_array_push(&ary, &n);
  SET_RETURN(ary);
}

/* See src/mrubyc/display.c in picoruby-tab5-display for how this name is derived. */
void
mrbc_tab5_touch_init(mrbc_vm *vm)
{
  mrbc_class *mrbc_class_Touch = mrbc_define_class(vm, "Touch", mrbc_class_object);

  mrbc_define_method(vm, mrbc_class_Touch, "point", c_Touch_point);
}
