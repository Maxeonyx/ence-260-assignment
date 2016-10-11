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

enum operator_enum {
    OP_ADD,
    OP_MUL,
    OP_SUB,
    OP_DIV
};

GameState game_state = START_SCREEN;


int sender_number_1 = 0;
int sender_operator = 0;
int sender_number_2 = 0;

char digit_base_10_to_char(int digit) {
    char chr = '0';
    chr += digit;
    return chr;
}

char * to_text(int num) {
    
    // we only want to deal with positive numbers in the range 0-99
    if (num < 0) {
        num = -(num % 100); // C's modulo isn't the mathematical one
    } else {
        num = num % 100;
    }
    
    char * str = "  ";
    
    if (num < 10) {
        // only use the second character if our number is only 1 digit
        str[1] = digit_base_10_to_char(num);
    } else {
        str[0] = digit_base_10_to_char(num / 10);
        str[1] = digit_base_10_to_char(num % 10);
    }
    
    return str;
}

char * to_operator(int operator) {
    
    switch (operator) {
        case OP_ADD: return " +";
        case OP_MUL: return " X";
        case OP_SUB: return " -";
        case OP_DIV: return " /";
    }
    
    return "NO";
}

void change_to_start_screen() {
    
    game_state = START_SCREEN;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  PRESS START!");
    
}

void change_to_choose_num_1() {
    
    game_state = CHOOSE_NUMBER_1;
    
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (to_text(sender_number_1));
    tinygl_text_speed_set (100);
    
}

void change_to_choose_num_2() {
    
    game_state = CHOOSE_NUMBER_2;
    
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (to_text(sender_number_2));
    tinygl_text_speed_set (100);
    
}


void change_to_choose_operator() {
    
    game_state = CHOOSE_OPERATOR;
    
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (to_operator(sender_operator));
    tinygl_text_speed_set (100);
    
}

void change_to_wait_for_send() {
    
    game_state = WAIT_FOR_SEND;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  PRESS BUT1 TO SEND!");
    
}


int add_modulo(int number, int amount, int modulo) {
    
    number += amount;
    
    number = number % modulo;
    if (number < 0) {
        number += modulo;
    }
    
    return number;
    
}


void number_select(int * int_var, int max_int, char* (*textFunction)(int)) {
    
    if (navswitch_push_event_p (NAVSWITCH_WEST)) {
        *int_var = add_modulo(*int_var, 1, max_int);
    }
    if (navswitch_push_event_p (NAVSWITCH_EAST)) {
        *int_var = add_modulo(*int_var, -1, max_int);
    }
    if (max_int > 10) {
        if (navswitch_push_event_p (NAVSWITCH_NORTH)) {
            *int_var = add_modulo(*int_var, 10, max_int);
        }
        if (navswitch_push_event_p (NAVSWITCH_SOUTH)) {
            *int_var = add_modulo(*int_var, -10, max_int);
        }
    }
    
    tinygl_text (textFunction(*int_var));
    
}


static void game_loop (__unused__ void *data) {
    
    tinygl_update ();
    
    button_update();
    navswitch_update();
    
    if (game_state == START_SCREEN) {
        
        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            change_to_choose_num_1();
        }
        
    } else if (game_state == CHOOSE_NUMBER_1) {
        
        number_select( &sender_number_1, 10, &to_text);
        
        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            change_to_choose_operator();
        }
        
    } else if (game_state == CHOOSE_OPERATOR) {
        
        number_select( &sender_operator, 4, &to_operator);
        
        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            change_to_choose_num_2();
        }
        
        
    } else if (game_state == CHOOSE_NUMBER_2) {
        
        number_select( &sender_number_2, 10, &to_text);
        
        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            change_to_wait_for_send();
        }
        
    } else if (game_state == WAIT_FOR_SEND) {
        
        if (button_push_event_p (BUTTON1)) {
            change_to_start_screen();
        }
        
    }
    
}


int main (void)
{
    system_init ();
    tinygl_init (DISPLAY_UPDATE_RATE);
    navswitch_init();
    button_init();
    
    change_to_start_screen();
    
    task_t tasks[] =
    {
        {.func = game_loop, .period = TASK_RATE / DISPLAY_UPDATE_RATE}
    };
    
    task_schedule (tasks, ARRAY_SIZE (tasks));
    
    return 0;
    
}