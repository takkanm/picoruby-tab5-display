#include "tab5_display.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_cache.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"

#include "bsp/esp-bsp.h"

static const char *TAG = "tab5_display";

static bsp_lcd_handles_t s_lcd = {0};
static SemaphoreHandle_t s_draw_done = NULL;
static bool s_ready = false;
static int s_rotation = 0;

/*
 * Scratch buffer for rotated draws. The pixels have to be rearranged before
 * draw_bitmap sees them, and the caller's buffer is const. It grows to the
 * largest draw seen so far and is never freed: the badge draws in strips of
 * at most a few tens of KB.
 */
static uint16_t *s_scratch = NULL;
static size_t s_scratch_len = 0;

/*
 * On a DPI panel esp_lcd_panel_draw_bitmap() returns before the transfer is
 * done. Submitting the next one without waiting fails with "previous draw
 * operation is not finished", so every draw waits on this semaphore.
 */
static bool
on_color_trans_done(esp_lcd_panel_handle_t panel,
                    esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
  BaseType_t higher_priority_task_woken = pdFALSE;
  xSemaphoreGiveFromISR((SemaphoreHandle_t)user_ctx, &higher_priority_task_woken);
  return higher_priority_task_woken == pdTRUE;
}

esp_err_t
tab5_display_init(void)
{
  if (s_ready) {
    return ESP_OK;
  }

  /*
   * The BSP header carries the nominal lane bit rate (1 Gbps); pass it
   * through. Lowering it stops the panel from syncing.
   */
  const bsp_display_config_t cfg = {
    .dsi_bus = {
      .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
      .lane_bit_rate_mbps = BSP_LCD_MIPI_DSI_LANE_BITRATE_MBPS,
    },
  };

  /*
   * This one call covers I2C, releasing the LCD and touch resets, DSI PHY
   * power, detecting the board generation, and creating the DSI bus and the
   * panel. The generation shows up in the log as
   * "Discovered board version N".
   */
  ESP_RETURN_ON_ERROR(bsp_display_new_with_handles(&cfg, &s_lcd), TAG, "bsp_display_new");

  s_draw_done = xSemaphoreCreateBinary();
  ESP_RETURN_ON_FALSE(s_draw_done, ESP_ERR_NO_MEM, TAG, "semaphore");

  const esp_lcd_dpi_panel_event_callbacks_t cbs = {
    .on_color_trans_done = on_color_trans_done,
  };
  ESP_RETURN_ON_ERROR(
    esp_lcd_dpi_panel_register_event_callbacks(s_lcd.panel, &cbs, s_draw_done),
    TAG, "register callbacks");

  /* The BSP deliberately leaves both the display and the backlight off. */
  ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(s_lcd.panel, true), TAG, "disp on");
  ESP_RETURN_ON_ERROR(bsp_display_brightness_set(100), TAG, "brightness");

  s_ready = true;
  ESP_LOGI(TAG, "panel ready: %dx%d RGB565", TAB5_DISPLAY_WIDTH, TAB5_DISPLAY_HEIGHT);
  return ESP_OK;
}

esp_err_t
tab5_display_set_brightness(int percent)
{
  return bsp_display_brightness_set(percent);
}

esp_err_t
tab5_display_set_rotation(int degrees)
{
  ESP_RETURN_ON_FALSE(degrees == 0 || degrees == 90 || degrees == 180 || degrees == 270,
                      ESP_ERR_INVALID_ARG, TAG, "rotation must be 0, 90, 180 or 270");
  s_rotation = degrees;
  return ESP_OK;
}

int
tab5_display_rotation(void)
{
  return s_rotation;
}

int
tab5_display_width(void)
{
  return (s_rotation == 90 || s_rotation == 270) ? TAB5_DISPLAY_HEIGHT : TAB5_DISPLAY_WIDTH;
}

int
tab5_display_height(void)
{
  return (s_rotation == 90 || s_rotation == 270) ? TAB5_DISPLAY_WIDTH : TAB5_DISPLAY_HEIGHT;
}

/*
 * Turn a logical w x h bitmap at (x, y) into the panel's frame: rearrange
 * the pixels into s_scratch and work out the physical rectangle.
 *
 * With W x H the physical size, a logical pixel (lx, ly) lands at
 *     90:  (W - 1 - ly, lx)
 *    180:  (W - 1 - lx, H - 1 - ly)
 *    270:  (ly, H - 1 - lx)
 */
static esp_err_t
rotate(int x, int y, int w, int h, const uint16_t *src,
       int *px, int *py, int *pw, int *ph)
{
  const size_t len = (size_t)w * h;
  if (s_scratch_len < len) {
    uint16_t *buf = heap_caps_realloc(s_scratch, len * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "no memory for a %dx%d rotation", w, h);
    s_scratch = buf;
    s_scratch_len = len;
  }

  uint16_t *dst = s_scratch;
  switch (s_rotation) {
  case 90:
    *px = TAB5_DISPLAY_WIDTH - y - h; *py = x; *pw = h; *ph = w;
    for (int ly = 0; ly < h; ly++) {
      for (int lx = 0; lx < w; lx++) {
        dst[lx * h + (h - 1 - ly)] = src[ly * w + lx];
      }
    }
    break;
  case 180:
    *px = TAB5_DISPLAY_WIDTH - x - w; *py = TAB5_DISPLAY_HEIGHT - y - h; *pw = w; *ph = h;
    for (size_t i = 0; i < len; i++) {
      dst[len - 1 - i] = src[i];
    }
    break;
  case 270:
    *px = y; *py = TAB5_DISPLAY_HEIGHT - x - w; *pw = h; *ph = w;
    for (int ly = 0; ly < h; ly++) {
      for (int lx = 0; lx < w; lx++) {
        dst[(w - 1 - lx) * h + ly] = src[ly * w + lx];
      }
    }
    break;
  }
  return ESP_OK;
}

/*
 * A full-screen fill writes straight into the DPI panel's frame buffer
 * instead of going through draw_bitmap, which avoids a 1.8 MB scratch
 * buffer. The frame buffer is cached, so the writes have to be flushed
 * afterwards.
 */
esp_err_t
tab5_display_fill(uint16_t rgb565)
{
  ESP_RETURN_ON_FALSE(s_ready, ESP_ERR_INVALID_STATE, TAG, "not initialized");

  void *fb = NULL;
  ESP_RETURN_ON_ERROR(esp_lcd_dpi_panel_get_frame_buffer(s_lcd.panel, 1, &fb),
                      TAG, "get frame buffer");

  const size_t count = (size_t)TAB5_DISPLAY_WIDTH * TAB5_DISPLAY_HEIGHT;
  uint16_t *px = (uint16_t *)fb;
  for (size_t i = 0; i < count; i++) {
    px[i] = rgb565;
  }

  return esp_cache_msync(fb, count * sizeof(uint16_t), ESP_CACHE_MSYNC_FLAG_DIR_C2M);
}

esp_err_t
tab5_display_draw(int x, int y, int w, int h, const uint16_t *pixels)
{
  ESP_RETURN_ON_FALSE(s_ready, ESP_ERR_INVALID_STATE, TAG, "not initialized");
  ESP_RETURN_ON_FALSE(pixels, ESP_ERR_INVALID_ARG, TAG, "pixels is NULL");
  ESP_RETURN_ON_FALSE(x >= 0 && y >= 0 && w > 0 && h > 0 &&
                      x + w <= tab5_display_width() && y + h <= tab5_display_height(),
                      ESP_ERR_INVALID_ARG, TAG,
                      "rect (%d,%d %dx%d) is out of the %dx%d screen",
                      x, y, w, h, tab5_display_width(), tab5_display_height());

  if (s_rotation != 0) {
    ESP_RETURN_ON_ERROR(rotate(x, y, w, h, pixels, &x, &y, &w, &h), TAG, "rotate");
    pixels = s_scratch;
  }

  /* draw_bitmap takes an exclusive end coordinate */
  ESP_RETURN_ON_ERROR(
    esp_lcd_panel_draw_bitmap(s_lcd.panel, x, y, x + w, y + h, pixels),
    TAG, "draw bitmap");

  /* Wait for the transfer before the caller frees or reuses `pixels` */
  ESP_RETURN_ON_FALSE(xSemaphoreTake(s_draw_done, pdMS_TO_TICKS(1000)) == pdTRUE,
                      ESP_ERR_TIMEOUT, TAG, "draw did not complete");
  return ESP_OK;
}
