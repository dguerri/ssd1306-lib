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
#ifndef SSD1306_ESP32_H
#define SSD1306_ESP32_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t *screen;
  int32_t rows, cols;
} ssd1306_framebuffer_t;


/* Initialize the sd1306 and return a new frame buffer */
ssd1306_framebuffer_t *ssd1306_new();

/* Free the framebuffer */
void ssd1306_free(ssd1306_framebuffer_t *screen);

/* Set display contrast
 * @param contrast Amount of contrast 0x00-0xFF 
 */
void ssd1306_display_contrast(uint8_t contrast);

/* Add a bitmap to the virtual screen, starting at the specified location 
 * @param framebuffer The device framebuffer
 * @param x Starting column
 * @param y Starting row
 * @param bitmap Bitmap to add
 * @param rows Bitmap width
 * @param cols Bitmap height 
 */
void ssd1306_add_bitmap(ssd1306_framebuffer_t *framebuffer, int32_t x, int32_t y, uint8_t *const bitmap, uint16_t rows, uint16_t cols);

/* Add a null terminated string to the virtual screen, starting at the specified location.
   Works with multiline as well 
 * @param framebuffer The device framebuffer
 * @param x Starting column
 * @param y Starting row
 * @param text Text to write into the framebuffer
 */
void ssd1306_add_text(ssd1306_framebuffer_t *framebuffer, int32_t x, int32_t y, char *const text);

/* Clear the vistual screen 
 * @param framebuffer The device framebuffer
 */
void ssd1306_clear(ssd1306_framebuffer_t *framebuffer);

/* Draw the virtual screen on the ssd1306
 * @param framebuffer The device framebuffer
 */
void ssd1306_draw(ssd1306_framebuffer_t *const framebuffer);

#endif /* SSD1306_ESP32_H */
