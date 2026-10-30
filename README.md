# picoruby-tab5-display

Drive the M5Stack Tab5 display from PicoRuby, and read its accelerometer.

```ruby
require "display"

Display.fill(0xffff)                      # white
Display.draw(x, y, w, h, rgb565_bytes)    # blit a bitmap
Display.brightness = 40                   # 0..100
Display.rotation = 90                     # landscape from now on

require "imu"
IMU.acceleration                          #=> [x, y, z] in g
```

The panel is portrait: **720 x 1280, RGB565**.

## What is in here

Three things that have to travel together:

| | |
| --- | --- |
| `mrbgems/picoruby-tab5-display/` | The mrbgem for the `Display` class and its VM binding |
| `mrbgems/picoruby-tab5-imu/` | The mrbgem for the `IMU` class (the BMI270 accelerometer) |
| `components/tab5_display/` | The ESP-IDF component that talks to the panel and the accelerometer |

Each gem is optional. Leave out the `conf.gem` line of one you do not need.

The gem alone is not self-contained. Its `ports/esp32/display.c` is compiled
by ESP-IDF rather than by picoruby's rake build, and the dependency on the
Tab5 BSP can only be declared in an ESP-IDF component's `idf_component.yml`.

The panel work itself is delegated to
[esp-bsp](https://github.com/espressif/esp-bsp)'s official Tab5 BSP
(`espressif/m5stack_tab5`), which detects the panel generation at runtime.
All three generations are handled: ILI9881C, ST7123 and ST7121.

## Requirements

- ESP-IDF v5.5 or later (developed against v5.5.5)
- An ESP-IDF project that already embeds PicoRuby, i.e. one with a
  `components/picoruby-esp32` — see
  [R2P2-ESP32](https://github.com/picoruby/R2P2-ESP32)
- An M5Stack Tab5

## Using it

Add this repository as a submodule:

```sh
git submodule add https://github.com/takkanm/picoruby-tab5-display.git external/picoruby-tab5-display
```

Point ESP-IDF at the component, **before** `project()` in your root
`CMakeLists.txt`:

```cmake
list(APPEND EXTRA_COMPONENT_DIRS external/picoruby-tab5-display/components)
```

Add the gem to picoruby's build config
(`components/picoruby-esp32/build_config/riscv-esp-*.rb`):

```ruby
conf.gem File.expand_path(
  '../../../external/picoruby-tab5-display/mrbgems/picoruby-tab5-display', __dir__)
conf.gem File.expand_path(
  '../../../external/picoruby-tab5-display/mrbgems/picoruby-tab5-imu', __dir__)
```

> **Floats on the ESP32-P4.** If the build config is FemtoRuby's
> (`riscv-esp-femtoruby.rb` from R2P2-ESP32), check that it passes
> `-march=rv32imafc_zicsr_zifencei` for `esp32p4`. Without it the
> toolchain defaults to an ISA with the D extension, `MRBC_USE_FLOAT=2`
> compiles to double-precision instructions, and the first Float the VM
> touches -- such as `IMU.acceleration` returning one -- panics with
> `Illegal instruction`. The P4's FPU is single-precision only.

Initialise the panel from `app_main`, before starting the VM:

```c
#include "tab5_display.h"

void app_main(void) {
  ESP_ERROR_CHECK(tab5_display_init());
  picoruby_esp32();
}
```

`Display` does not expose an `init`. Initialisation order is the
application's business, and letting Ruby get it wrong is not worth the
convenience.

> A submodule rather than `conf.gem github:`. mruby's build system can fetch
> a gem from a remote, but it does so during the rake build, into
> `picoruby/build/mrbgems/`. The gem's ESP32 port has to exist when CMake
> configures, which is earlier than that.

## API

### Display

| | |
| --- | --- |
| `Display.width` | 720, or 1280 when rotated by 90 / 270 |
| `Display.height` | 1280, or 720 when rotated by 90 / 270 |
| `Display.fill(rgb565)` | Fill the screen with one colour |
| `Display.draw(x, y, w, h, pixels)` | Blit `pixels` at (x, y) |
| `Display.brightness = n` | Backlight, 0..100 |
| `Display.rotation = deg` | Rotate what is drawn from now on, clockwise: 0, 90, 180, 270 |
| `Display.rotation` | The current rotation |

`pixels` is a binary String of RGB565 data, little-endian and row-major.
Its length must be exactly `w * h * 2`; anything else raises
`ArgumentError`. A rectangle that leaves the screen raises `RangeError`.
Both are checked before drawing, because letting them through would read
past the end of the frame buffer.

Large images are best sent in horizontal strips, so that only one strip is
held in Ruby at a time:

```ruby
STRIP_ROWS = 16

row = 0
while row < h
  rows = h - row
  rows = STRIP_ROWS if STRIP_ROWS < rows
  Display.draw(x, y + row, w, rows, io.read(w * rows * 2))
  row += rows
end
```

#### Rotation

`Display.rotation = 90` makes everything drawn afterwards appear upright when
the Tab5 lies on its left side -- USB-C on the right. 270 is the other side,
180 is upside down. Coordinates passed to `draw` are in the rotated frame, and
`width` / `height` follow it.

Pixels already on screen are not moved: redraw after changing the rotation.
Rotated draws are rearranged in C through a scratch buffer, so there is no
Ruby-side cost; the scratch buffer grows to the largest draw seen.

### IMU

| | |
| --- | --- |
| `IMU.acceleration` | `[x, y, z]` in g, as Floats |

The sensor is set up on the first call. Measured on a Tab5, the axis that
reads about 1 g in each pose is:

| Pose | Axis | `Display.rotation` that keeps the picture upright |
| --- | --- | --- |
| Portrait, USB-C at the bottom | +y | 0 |
| Upside down | -y | 180 |
| Landscape, USB-C on the right | -x | 90 |
| Landscape, USB-C on the left | +x | 270 |
| Lying flat, screen up | -z | -- |

Orientation comes from the accelerometer, not the gyroscope: gravity gives
the direction directly, while integrating the gyroscope's rotation rate
drifts.

> The esp-bsp BMI270 driver waits `pdMS_TO_TICKS(10)` after its soft reset.
> At `CONFIG_FREERTOS_HZ=100` that is one tick, anything from 0 to 10 ms,
> and the next read sometimes fails with `ESP_ERR_INVALID_STATE`. The
> component retries the initialisation, so a warning like
> `BMI270 init failed ... retrying` on boot is expected.

### Notes

> Beware: `irb` in recent PicoRuby does not print exceptions. An expression
> that should raise looks like it returned something instead. Wrap it in
> `begin`/`rescue` when checking.

## Example

[takkanm/picoruby-tab5](https://github.com/takkanm/picoruby-tab5) uses this
as a conference name badge that follows the device's orientation, reading
its images from the device's own filesystem.

## License

MIT. See [LICENSE](LICENSE).
