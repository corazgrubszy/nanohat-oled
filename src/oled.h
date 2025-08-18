#pragma once

// commonly used offsets
#define LINE1	  0
#define LINE2	256
#define LINE3	512
#define LINE4	768

// functions
int oled_init();
int oled_turn_on_off(int state);
int oled_redraw();
void oled_clear_buffer();
void oled_draw_pixel(int x, int y);
void oled_draw_char(int row, int col, unsigned char *font, int offset);
void oled_print(char *str, int offset);

// new graphics API
void oled_draw_bitmap_xy(int x, int y, int width, int height, const unsigned char *bitmap);
void oled_draw_text_xy(int x, int y, const char *str);

// access raw internal buffer
unsigned char *oled_get_buffer();