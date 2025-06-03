/*
GUI

GUI-related stuff like text drawing, buttons etc.
*/

#pragma once

#include "../computer.h"
#include "../backend/math2d.h"
#include "../backend/gfx.h"

#define GUI_STANDARD_BUTTON_SIZE 16
#define GUI_SMALL_BUTTON_SIZE 12
#define GUI_BORDER_WIDTH 2

void gui_draw_string(ram_t *ram, int font_index, string_t string, point_t pos, int color);

void gui_draw_text(ram_t *ram, int font_index, const char text[], point_t pos, int color);

// Draw inside the specified area
void gui_outset_frame(ram_t *ram, rect_t rect);

// Draw around the specified area
void gui_inset_frame(ram_t *ram, rect_t rect);

bool gui_button_ex(ram_t *ram, char text[], rect_t rect, bool already_pressed);

bool gui_button(ram_t *ram, char text[], rect_t rect);

bool gui_press_button(ram_t *ram, char text[], rect_t rect);

bool gui_toggle_button(ram_t *ram, char text[], rect_t rect, bool set);

int gui_get_string_width(font_t *font, string_t string, int max_offset);

int gui_get_text_width(font_t *font, char text[], int max_offset);

// int gui_x_to_text_index(font_t *font, char text[], int x);
int gui_x_to_string_index(font_t *font, string_t string, int x);

void gui_init_font(ram_t *ram, int index, int start_sprite_index, int horizontal_space, int height, int h_sprites, int v_sprites);

rect_t gui_rect_to_outset_frame_rect(rect_t rect);

bool static_button(framebuffer_t *fb, surface_t src, point_t pos, rect_t unpressed_rect, rect_t pressed_rect, bool already_pressed);