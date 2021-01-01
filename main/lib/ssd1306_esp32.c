/* Copyright (c) 2020 Davide Guerri <davide.guerri@gmail.com>
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include "font8x8_basic.h"
#include "ssd1306_esp32.h"
#include "ssd1306_utils.h"
#include "ssd1366.h"

#define FRAMEBUFFER_WIDTH 128
#define FRAMEBUFFER_HEIGHT 64

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT FRAMEBUFFER_HEIGHT / 8

#define SDA_PIN GPIO_NUM_4
#define SCL_PIN GPIO_NUM_15
#define RST_PIN GPIO_NUM_16

#define tag "SSD1306"

void i2c_master_init() {
  i2c_config_t i2c_config = {.mode = I2C_MODE_MASTER,
                             .sda_io_num = SDA_PIN,
                             .scl_io_num = SCL_PIN,
                             .sda_pullup_en = GPIO_PULLUP_ENABLE,
                             .scl_pullup_en = GPIO_PULLUP_ENABLE,
                             .master.clk_speed = 700000};
  i2c_param_config(I2C_NUM_0, &i2c_config);
  i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void ssd1306_init() {
  esp_err_t espRc;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(RST_PIN, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level(RST_PIN, 1);

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

  i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
  i2c_master_write_byte(cmd, 0x14, true);

  i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP,
                        true); // reverse left-right mapping
  i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE,
                        true); // reverse up-bottom mapping

  i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
  i2c_master_stop(cmd);

  espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
  if (espRc == ESP_OK) {
    ESP_LOGI(tag, "OLED configured successfully");
  } else {
    ESP_LOGE(tag, "OLED configuration failed. code: 0x%.2X", espRc);
  }
  i2c_cmd_link_delete(cmd);
}

void ssd1306_display_clear() {
  i2c_cmd_handle_t cmd;

  uint8_t zero[128];
  memset(zero, 0, sizeof zero);
  for (uint8_t i = 0; i < 8; i++) {
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
                          true);
    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
    i2c_master_write_byte(cmd, 0xB0 | i, true);

    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
    i2c_master_write(cmd, zero, 128, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
  }
}

void ssd1306_display_contrast(uint8_t contrast) {
  i2c_cmd_handle_t cmd;

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
  i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
  i2c_master_write_byte(cmd, contrast, true);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
}

ssd1306_framebuffer_t *ssd1306_new() {
  i2c_master_init();
  ssd1306_init();

  ssd1306_display_clear();
  vTaskDelay(100 / portTICK_PERIOD_MS);

  ssd1306_framebuffer_t *framebuffer =
      (ssd1306_framebuffer_t *)calloc(1, sizeof(ssd1306_framebuffer_t));
  framebuffer->cols = FRAMEBUFFER_WIDTH;
  framebuffer->rows = FRAMEBUFFER_HEIGHT;
  framebuffer->screen =
      (uint8_t *)calloc(framebuffer->rows * framebuffer->cols, sizeof(uint8_t));

  return framebuffer;
}

void ssd1306_free(ssd1306_framebuffer_t *framebuffer) {
  free(framebuffer->screen);
  free(framebuffer);
}

void ssd1306_add_bitmap(ssd1306_framebuffer_t *framebuffer, int32_t x,
                        int32_t y, uint8_t *const bitmap, uint16_t rows,
                        uint16_t cols) {
  int32_t bb_x, bb_y, bb_w, bb_h; // bounding box
  int32_t bm_x, bm_y;             // Bitmap offset

  if (x >= 0) {
    bb_x = x;
    bm_x = 0;
  } else {
    bb_x = 0;
    bm_x = -x;
  }
  if ((bb_w = min(framebuffer->cols, x + (int32_t)cols) - bb_x) <= 0)
    return;

  if (y >= 0) {
    bb_y = y;
    bm_y = 0;
  } else {
    bb_y = 0;
    bm_y = -y;
  }
  if ((bb_h = min(framebuffer->rows, y + (int32_t)rows) - bb_y) <= 0)
    return;

  for (int32_t i = 0; i < bb_h; i++) {
    memcpy(framebuffer->screen + (bb_y + i) * framebuffer->cols + bb_x,
           bitmap + (bm_y + i) * cols + bm_x, bb_w);
  }
}

void ssd1306_add_text(ssd1306_framebuffer_t *framebuffer, int32_t x, int32_t y,
                      char *const text) {
  int32_t col = x;
  int32_t row = y;

  for (int32_t i = 0; i < strlen(text); i++) {
    if (text[i] == '\n') {
      if ((row += 8) > framebuffer->rows)
        return;
      col = x;
    } else if (col < framebuffer->cols && row + 8 > 0) {
      if (col + 8 >= 0) {
        for (int32_t j = 0; j < 8 && row + j < framebuffer->rows; j++) {
          if (row + j > 0) {
            memcpy(framebuffer->screen + (row + j) * framebuffer->cols +
                       max(0, col),
                   font8x8_basic_tr + text[i] * 64 + j * 8 - min(0, col),
                   min(8, framebuffer->cols - col));
          }
        }
      }
      col += 8;
    }
  }
}

void ssd1306_clear(ssd1306_framebuffer_t *framebuffer) {
  memset(framebuffer->screen, 0, framebuffer->rows * framebuffer->cols);
}

void ssd1306_draw(ssd1306_framebuffer_t *const framebuffer) {
  i2c_cmd_handle_t cmd;
  uint8_t line[SCREEN_WIDTH];

  for (uint8_t row = 0; row < SCREEN_HEIGHT; row++) {
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
                          true);
    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
    i2c_master_write_byte(cmd, 0xB0 | row, true);

    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
    for (uint8_t col = 0; col < SCREEN_WIDTH; col++) {
      line[col] = 0;
      for (uint8_t j = 0; j < 8; j++) {
        line[col] |=
            *(framebuffer->screen + (row * 8 + j) * framebuffer->cols + col)
            << (j);
      }
    }
    i2c_master_write(cmd, line, SCREEN_WIDTH, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
  }
}
