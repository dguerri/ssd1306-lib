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
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "sdkconfig.h" // generated by "make menuconfig"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lib/ssd1306_esp32.h"
#include "lib/ssd1306_utils.h"

#include "mario.h"

#define STACK_SIZE 2048

void task_contrast(void *_) {
  uint8_t c = 0;
  int8_t a = 1;

  while (true) {
    ssd1306_display_contrast(c);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    if (c == 0x00)
      a = 1;
    if (c == 0xFF)
      a = -1;
    c += a;
  }

  vTaskDelete(NULL);
}

void task_demo(void *fb) {
  ssd1306_framebuffer_t *framebuffer = (ssd1306_framebuffer_t *)fb;
  int32_t mario_x = -MARIO_W, mario_y = framebuffer->rows - MARIO_H / 2;
  int32_t mario_y_dir = 1;

  int32_t text_x = -64, text_y = -12;
  int32_t text_x_dir = 1, text_y_dir = 1;

  while (1) {
    ssd1306_clear(framebuffer);
    ssd1306_add_text(framebuffer, text_x, text_y,
                     "The quick brown\nfox jumps over\nthe lazy dog!");
    ssd1306_add_bitmap(framebuffer, mario_x, mario_y, mario, MARIO_W, MARIO_H);
    ssd1306_draw(framebuffer);

    if (mario_x > framebuffer->cols + MARIO_W) {
      mario_x = -MARIO_W;
      mario_y = framebuffer->rows - MARIO_H / 2;
    }
    if (mario_y > framebuffer->rows - MARIO_H / 2)
      mario_y_dir = -1;
    else if (mario_y == -MARIO_H / 2)
      mario_y_dir = 1;

    mario_x++;
    mario_y += mario_y_dir;

    if (text_x > framebuffer->cols - 64)
      text_x_dir = -1;
    else if (text_x <= -64)
      text_x_dir = 1;
    if (text_y > framebuffer->rows - 12)
      text_y_dir = -1;
    else if (text_y <= -12)
      text_y_dir = 1;

    text_x += text_x_dir;
    text_y += text_y_dir;
  }

  vTaskDelete(NULL);
}

void app_main(void) {
  ssd1306_framebuffer_t *framebuffer = ssd1306_new();

  // xTaskCreate(&task_contrast, "contrast", STACK_SIZE, NULL,
  //            configMAX_PRIORITIES - 3, NULL);
  xTaskCreate(&task_demo, "demo", STACK_SIZE, framebuffer,
              configMAX_PRIORITIES - 1, NULL);
}
