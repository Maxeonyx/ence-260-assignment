/**

    our_display.h
    Ryely Burtenshaw-Day, Maxwell Clarke

*/

#include "tinygl.h"

#ifndef TEAM32_DISPLAY_H
#define TEAM32_DISPLAY_H


#ifndef TEXT_SCROLL_SPEED
#define TEXT_SCROLL_SPEED 28
#endif

/* set the tinygl settings to scroll text */
void set_scroll_text (void);

/* set the tinygl settings for screens like the number chooser*/
void set_static_text (void);


#endif