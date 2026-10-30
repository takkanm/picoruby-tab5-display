# The Tab5 panel, as an ESP-IDF component.
#
# It is a component of its own rather than part of the application, because
# libmruby (linked into picoruby-esp32) references Display_show_* symbols
# defined here. Putting this inside the application's `main` component would
# make picoruby-esp32 depend on `main`, which together with main's own
# dependency on picoruby-esp32 is a cycle.
idf_component_register(
  SRCS
    "tab5_display.c"
    # The ESP32 port of the picoruby-tab5-display gem.
    #
    # It is gem code, but it includes only esp_log.h and tab5_display.h --
    # no mruby headers -- so it can be compiled here instead of inside
    # picoruby-esp32. That keeps the change needed in picoruby-esp32 down to
    # a single conf.gem line in its build_config.
    "../../mrbgems/picoruby-tab5-display/ports/esp32/display.c"
    # The accelerometer, and the ESP32 port of the picoruby-tab5-imu gem,
    # for the same reasons.
    "tab5_imu.c"
    "../../mrbgems/picoruby-tab5-imu/ports/esp32/imu.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES esp_lcd esp_mm
)
