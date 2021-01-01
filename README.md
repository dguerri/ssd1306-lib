# SSD1306 64x128 on I2C C library for ESP-IDF apps

## WiP

Not yet a proper library, but this repo contains code to run the ssd1306 oled display on I2C for ESP32.
I am targeting this at the [Heltec ESP32](https://heltec.org/project/wifi-kit-32/).

![Alt Text](doc/ssd1306-demo.gif)

## API
The API is super simple:

```C
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
```