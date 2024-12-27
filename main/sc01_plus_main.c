/*
 * ST7796 via i80 demo on SC01 PLUS...
 * using ESP-IDF v5.3.1 (new i80 interface)
 * and esp_lcd_st7796 component from espressif
 * 27.12.2024 by mathertel
 *
 * With help from: (links may be broken in the future)
 * * <https://en.wireless-tag.com/product-item-26.html>
 * * <https://github.com/espressif/esp-idf/issues/12347>
 *
 * Example code:ST7796:
 * * https://github.com/espressif/esp-bsp/tree/master/components/lcd/esp_lcd_st7796
 * * https://components.espressif.com/components/espressif/esp_lcd_st7796/versions/1.3.1
 *
 * sc01 plus board parameters:
 * * https://doc.riot-os.org/group__boards__esp32s3__wt32__sc01__plus.html
 * * https://github.com/sukesh-ak/BSP-IDF5-ESP_LCD-LVGL9/blob/6f95b0961ec80f674e2e4c3abd350aa85d06c173/components/wt32sc01plus/include/bsp/display.h
 * * https://github.com/sukesh-ak/BSP-IDF5-ESP_LCD-LVGL9/blob/6f95b0961ec80f674e2e4c3abd350aa85d06c173/components/wt32sc01plus/wt32sc01plus.c#L198
 *
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_io_i80.h"
#include "esp_lcd_types.h"
#include "esp_lcd_st7796.h"

#include "driver/gpio.h"

// Logging prefix
static const char *TAG = "ST7796";

esp_lcd_panel_handle_t panel_handle = NULL;

void init_lcd() {
  ESP_LOGI(TAG, "Initialize Intel 8080 bus");
  esp_lcd_i80_bus_handle_t i80_bus = NULL;
  esp_lcd_i80_bus_config_t bus_config = {
    .clk_src = LCD_CLK_SRC_PLL160M,
    .dc_gpio_num = (GPIO_NUM_0),
    .wr_gpio_num = (GPIO_NUM_47),
    .data_gpio_nums = {
      (GPIO_NUM_9), (GPIO_NUM_46), (GPIO_NUM_3), (GPIO_NUM_8), (GPIO_NUM_18), (GPIO_NUM_17), (GPIO_NUM_16), (GPIO_NUM_15) },
    .bus_width = 8,
    .max_transfer_bytes = 4096,
    .psram_trans_align = 64,
    .sram_trans_align = 4,
  };

  ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

  ESP_LOGI(TAG, "Install panel IO");
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_i80_config_t io_config =  // ST7796_PANEL_IO_I80_CONFIG(EXAMPLE_PIN_NUM_LCD_CS, example_callback, &example_callback_ctx);
    {
      .cs_gpio_num = GPIO_NUM_NC,
      .pclk_hz = 10 * 1000 * 1000,
      .trans_queue_depth = 10,
      .dc_levels = {
        .dc_idle_level = 0,
        .dc_cmd_level = 0,
        .dc_dummy_level = 0,
        .dc_data_level = 1,
      },
      .flags = {
        .swap_color_bytes = 1,
        .pclk_idle_low = 0,
      },
      .lcd_cmd_bits = 8,
      .lcd_param_bits = 8,
    };

  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

  ESP_LOGI(TAG, "Install ST7796 panel driver");
  const esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = (GPIO_NUM_4),  // Set to -1 if not use
    .rgb_endian = LCD_RGB_ENDIAN_RGB,
    .bits_per_pixel = 16,  // Implemented by LCD command `3Ah` (16/18/24)
                           // .vendor_config = &vendor_config,            // Uncomment this line if use custom initialization commands
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
  esp_lcd_panel_invert_color(panel_handle, true);
  esp_lcd_panel_mirror(panel_handle, true, false);
}


void fill_rect(int x_pos, int y_pos, int w, int h, int16_t color) {
  uint16_t *img = heap_caps_malloc(2 * w, MALLOC_CAP_DMA);
  for (int x = 0; x < w; x++) {
    img[x] = color;
  }

  for (int y = y_pos; y < y_pos + h; y++) {
    esp_lcd_panel_draw_bitmap(panel_handle, x_pos, y, x_pos + w, y + 1, img);
  }
  heap_caps_free(img);
}


void app_main(void) {
  printf("ST7796 via i80 demo on SC01 PLUS...\n");

  /* Print chip information */
  esp_chip_info_t chip_info;
  uint32_t flash_size;
  esp_chip_info(&chip_info);
  printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
         CONFIG_IDF_TARGET,
         chip_info.cores,
         (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
         (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
         (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
         (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

  unsigned major_rev = chip_info.revision / 100;
  unsigned minor_rev = chip_info.revision % 100;
  printf("silicon revision v%d.%d, ", major_rev, minor_rev);
  if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
    printf("Get flash size failed");
    return;
  }

  printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

  gpio_config_t bk_gpio_config = {
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1ULL << (GPIO_NUM_45)
  };
  ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
  ESP_ERROR_CHECK(gpio_set_level((GPIO_NUM_45), 1));

  // initialize i80 and ST7796 panel
  init_lcd();

  // blank screen
  fill_rect(0, 0, 320, 480, 0xFFFF);

  // some random rectangles...
  TickType_t pause = 15 / portTICK_PERIOD_MS;
  printf("Pause %ld\n", pause);

  for (int n = 0; n < 120; n++) {
    uint16_t ran_color = esp_random() & 0xFFFF;
    uint16_t ran_x = esp_random() % (320 - 60);
    uint16_t ran_y = esp_random() % (480 - 60);
    fill_rect(ran_x, ran_y, 60, 60, ran_color);
    vTaskDelay(pause);
  }

  for (int i = 10; i >= 0; i--) {
    printf("Restarting in %d seconds...\n", i);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  printf("Restarting now.\n");
  fflush(stdout);
  esp_restart();
}
