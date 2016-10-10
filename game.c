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
int sender_number_2 = 0;
int sender_operator = 0;


//number options
char * to_text(int num) {

    if (num < 10 && num >= 0) {
        char* str = " 0";
        str[1] += num;
        return str;
    }

    return "HI";
}
//operator options
char * to_operator(int operator) {

    switch (operator) {
        case 0: return " +";
        case 1: return " X";
        case 2: return " -";
        case 3: return " /";
    }

    return "NO";
}

//Update the display
static void display_task (__unused__ void *data) {
    tinygl_update ();
}

//Set up the start screen to scroll through "PRESS START" on repeat until the navswitch is pressed down.
void change_to_start_screen() {

    curr_state = START_SCREEN;

    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  PRESS START!");
    tinygl_text_speed_set (20);

}

void change_to_choose_num() {
    //Fixing bug where pushing the navswitch in the E direction when choosing number two returned "HI"
    if (curr_state == CHOOSE_NUMBER_1 || curr_state == START_SCREEN){
        curr_state = CHOOSE_NUMBER_1;
    }
    else {
        curr_state = CHOOSE_NUMBER_2;
    }

    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (" 0");
    tinygl_text_speed_set (100);

}

static void game_loop (__unused__ void *data) {

    navswitch_update();
    //Display the starting screen until the navswitch is pressed down.
    if (curr_state == START_SCREEN) {

        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            change_to_choose_num();

        }

    } else if (curr_state == CHOOSE_NUMBER_1) {


        //Count upwards if the navswitch is pushed in the W direction.
        if (navswitch_push_event_p (NAVSWITCH_WEST)) {
            sender_number_1 = (sender_number_1 + 1) % 10;
            tinygl_text (to_text(sender_number_1));
        }
        //Count downwards if the navswitch is pushed in the E direction.
        if (navswitch_push_event_p (NAVSWITCH_EAST)) {
            sender_number_1 = (sender_number_1 - 1);
            if (sender_number_1 < 0) {
                sender_number_1 = 9;
            }
            tinygl_text (to_text(sender_number_1));
        }
        //Once the navswitch is pushed down set the current state to choosing the operator.
        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            curr_state = CHOOSE_OPERATOR;
            tinygl_clear();
        }

    } else if (curr_state == CHOOSE_OPERATOR) {
        
        //Display first operator.
        tinygl_text (to_operator(sender_operator));
        //Go forwards in the order of operators if the navswitch is pushed in the W direction.
        if (navswitch_push_event_p (NAVSWITCH_WEST)) {
            sender_operator = (sender_operator + 1) % 4;
            //tinygl_text (to_operator(sender_operator));
        }
        //Go backwards in the order of operators if the navswitch is pushed in the E direction.
        if (navswitch_push_event_p (NAVSWITCH_EAST)) {
            sender_operator = (sender_operator - 1);
            if (sender_operator < 0) {
                sender_operator = 3;
            }
            //tinygl_text (to_operator(sender_operator));
        }
        //Once the navswitch is pushed down set the current state to choosing the second number.
        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            curr_state = CHOOSE_NUMBER_2;
            tinygl_clear();
        }


    } else if (curr_state == CHOOSE_NUMBER_2) {
        //Display first number.
        tinygl_text (to_text(sender_number_2));
        
        //Count upwards if the navswitch is pushed in the W direction.
        if (navswitch_push_event_p (NAVSWITCH_WEST)) {
            sender_number_2 = (sender_number_2 + 1) % 10;
            //led_set (LED1, 1);
            //tinygl_text (to_text(sender_number_2));
        }
        //Count downwards if the navswitch is pushed in the E direction.
        if (navswitch_push_event_p (NAVSWITCH_EAST)) {
            sender_number_2 = (sender_number_2 - 1);
            if (sender_number_2 < 0) {
                sender_number_2 = 9;
            }
            //tinygl_text (to_text(sender_number_1));
        }
        //Once the navswitch is pushed down wait for the send command.
        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            curr_state = WAIT_FOR_SEND;
            led_set (LED1, 1);
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