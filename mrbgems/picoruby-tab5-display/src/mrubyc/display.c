#include <mrubyc.h>
#include "../../include/display.h"

static void
c_Display_width(mrbc_vm *vm, mrbc_value *v, int argc)
{
  SET_INT_RETURN(Display_width());
}

static void
c_Display_height(mrbc_vm *vm, mrbc_value *v, int argc)
{
  SET_INT_RETURN(Display_height());
}

static void
c_Display_fill(mrbc_vm *vm, mrbc_value *v, int argc)
{
  if (argc != 1) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments. expected 1");
    return;
  }
  if (v[1].tt != MRBC_TT_INTEGER) {
    mrbc_raise(vm, MRBC_CLASS(TypeError), "wrong argument type. expected Integer");
    return;
  }
  int color = GET_INT_ARG(1);
  if (color < 0 || 0xffff < color) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "color must be within 0..0xffff");
    return;
  }
  if (!Display_fill((uint16_t)color)) {
    mrbc_raise(vm, MRBC_CLASS(RuntimeError), "fill failed");
    return;
  }
  SET_TRUE_RETURN();
}

/*
 * Display.draw(x, y, w, h, pixels)
 *
 * `pixels` is a binary String of RGB565 data. A length other than
 * w * h * 2, or a rectangle that leaves the screen, is rejected before
 * drawing: letting either through would make the C side read past the end
 * of the frame buffer.
 */
static void
c_Display_draw(mrbc_vm *vm, mrbc_value *v, int argc)
{
  if (argc != 5) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments. expected 5");
    return;
  }
  for (int i = 1; i <= 4; i++) {
    if (v[i].tt != MRBC_TT_INTEGER) {
      mrbc_raise(vm, MRBC_CLASS(TypeError), "x, y, w, h must be Integer");
      return;
    }
  }
  if (v[5].tt != MRBC_TT_STRING) {
    mrbc_raise(vm, MRBC_CLASS(TypeError), "pixels must be String");
    return;
  }

  int x = GET_INT_ARG(1);
  int y = GET_INT_ARG(2);
  int w = GET_INT_ARG(3);
  int h = GET_INT_ARG(4);

  if (w <= 0 || h <= 0 || x < 0 || y < 0 ||
      x + w > Display_width() || y + h > Display_height()) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "rect is out of the screen");
    return;
  }

  int expected = w * h * 2;
  int len = mrbc_string_size(&v[5]);
  if (len != expected) {
    mrbc_raisef(vm, MRBC_CLASS(ArgumentError),
                "pixels must be %d bytes for %dx%d, got %d", expected, w, h, len);
    return;
  }

  if (!Display_draw(x, y, w, h, (const uint8_t *)v[5].string->data)) {
    mrbc_raise(vm, MRBC_CLASS(RuntimeError), "draw failed");
    return;
  }
  SET_TRUE_RETURN();
}

static void
c_Display_brightness_eq(mrbc_vm *vm, mrbc_value *v, int argc)
{
  if (argc != 1) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments. expected 1");
    return;
  }
  if (v[1].tt != MRBC_TT_INTEGER) {
    mrbc_raise(vm, MRBC_CLASS(TypeError), "wrong argument type. expected Integer");
    return;
  }
  int percent = GET_INT_ARG(1);
  if (percent < 0 || 100 < percent) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "brightness must be within 0..100");
    return;
  }
  if (!Display_set_brightness(percent)) {
    mrbc_raise(vm, MRBC_CLASS(RuntimeError), "brightness failed");
    return;
  }
  SET_INT_RETURN(percent);
}

/*
 * The name of this function is derived from the GEM name, not from
 * require_name. picoruby-require calls "mrbc_" + the gem name with the
 * "picoruby-" prefix removed and hyphens turned into underscores + "_init".
 * So picoruby-tab5-display gives mrbc_tab5_display_init.
 * spec.require_name ("display") has nothing to do with it.
 */
void
mrbc_tab5_display_init(mrbc_vm *vm)
{
  mrbc_class *mrbc_class_Display = mrbc_define_class(vm, "Display", mrbc_class_object);

  mrbc_define_method(vm, mrbc_class_Display, "width", c_Display_width);
  mrbc_define_method(vm, mrbc_class_Display, "height", c_Display_height);
  mrbc_define_method(vm, mrbc_class_Display, "fill", c_Display_fill);
  mrbc_define_method(vm, mrbc_class_Display, "draw", c_Display_draw);
  mrbc_define_method(vm, mrbc_class_Display, "brightness=", c_Display_brightness_eq);
}
