#include "system.h"
#include "tinygl.h"
#include "task.h"
#include "button.h"
#include "ledmat.h"
#include "pio.h"
#include "led.h"
#include "navswitch.h"
#include "../../fonts/font3x5_1.h"

#define DISPLAY_UPDATE_RATE 1000

typedef enum game_state_enum {
    START_SCREEN,

    //sender
    CHOOSE_NUMBER_1,
    CHOOSE_OPERATOR,
    CHOOSE_NUMBER_2,
    WAIT_FOR_SEND,

    //receiver
    DISPLAY_QUESTION,
    CHOOSE_ANSWER,
    DISPLAY_RESULT
} GameState;

GameState curr_state = START_SCREEN;

int sender_number_1 = 0;
int sender_operator = 0;

char * to_text(int num) {

    if (num < 10 && num >= 0) {
        char* str = " 0";
        str[1] += num;
        return str;
    }

    return "HI";
}

char * to_operator(int operator) {

    switch (operator) {
        case 0: return " +";
        case 1: return " X";
        case 2: return " -";
        case 3: return " |";
    }

    return "NO";
}

static void display_task (__unused__ void *data) {
    tinygl_update ();
}

void change_to_start_screen() {

    curr_state = START_SCREEN;

    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  PRESS START!");
    tinygl_text_speed_set (20);

}

void change_to_choose_num() {

    curr_state = CHOOSE_NUMBER_1;

    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (" 0");
    tinygl_text_speed_set (100);

}

static void game_loop (__unused__ void *data) {

    navswitch_update();

    if (curr_state == START_SCREEN) {

        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            change_to_choose_num();

        }

    } else if (curr_state == CHOOSE_NUMBER_1) {


        if (navswitch_push_event_p (NAVSWITCH_WEST)) {
            sender_number_1 = (sender_number_1 + 1) % 10;
            led_set (LED1, 1);
            tinygl_text (to_text(sender_number_1));
        }
        if (navswitch_push_event_p (NAVSWITCH_EAST)) {
            sender_number_1 = (sender_number_1 - 1);
            if (sender_number_1 < 0) {
                sender_number_1 = 9;
            }
            tinygl_text (to_text(sender_number_1));
        }
        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            curr_state = CHOOSE_OPERATOR;
        }

    } else if (curr_state = CHOOSE_OPERATOR) {

        if (navswitch_push_event_p (NAVSWITCH_WEST)) {
            sender_operator = (sender_operator + 1) % 4;
            led_set (LED1, 1);
            tinygl_text (to_operator(sender_operator));
        }
        if (navswitch_push_event_p (NAVSWITCH_EAST)) {
            sender_operator = (sender_operator - 1);
            if (sender_operator < 0) {
                sender_operator = 3;
            }
            tinygl_text (to_operator(sender_operator));
        }


    }

}


int main (void)
{
    system_init ();
    tinygl_init (DISPLAY_UPDATE_RATE);
    navswitch_init();
    
    change_to_start_screen();

    task_t tasks[] =
    {
      {.func = display_task, .period = TASK_RATE / DISPLAY_UPDATE_RATE},
      {.func = game_loop, .period = TASK_RATE / DISPLAY_UPDATE_RATE}
    };

    task_schedule (tasks, ARRAY_SIZE (tasks));

    return 0;

}