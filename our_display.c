/**

    our_display.c
    Ryely Burtenshaw-Day, Maxwell Clarke

*/

#include "our_display.h"

#include "tinygl.h"

void set_scroll_text (void) {
    tinygl_text_speed_set (TEXT_SCROLL_SPEED);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
}

void set_static_text (void) {
    tinygl_text_speed_set (100);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
}