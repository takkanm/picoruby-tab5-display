# picoruby-tab5-display

Drive the M5Stack Tab5 display from PicoRuby.

```ruby
require "display"

Display.fill(0xffff)                      # white
Display.draw(x, y, w, h, rgb565_bytes)    # blit a bitmap
Display.brightness = 40                   # 0..100
```

The panel is portrait: **720 x 1280, RGB565**.

## What is in here

Two things that have to travel together:

| | |
| --- | --- |
| `mrbgems/picoruby-tab5-display/` | The mrbgem: the `Display` class and its VM binding |
| `components/tab5_display/` | The ESP-IDF component that talks to the panel |

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
```

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

| | |
| --- | --- |
| `Display.width` | 720 |
| `Display.height` | 1280 |
| `Display.fill(rgb565)` | Fill the screen with one colour |
| `Display.draw(x, y, w, h, pixels)` | Blit `pixels` at (x, y) |
| `Display.brightness = n` | Backlight, 0..100 |

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

> Beware: `irb` in recent PicoRuby does not print exceptions. An expression
> that should raise looks like it returned something instead. Wrap it in
> `begin`/`rescue` when checking.

## Example

[takkanm/picoruby-tab5](https://github.com/takkanm/picoruby-tab5) uses this
to show a logo on power-up, reading the image from the device's own
filesystem.

## License

MIT. See [LICENSE](LICENSE).
