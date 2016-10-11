#include "system.h"
#include "tinygl.h"
#include "task.h"
#include "button.h"
#include "ledmat.h"
#include "pio.h"
#include "led.h"
#include "navswitch.h"
#include "ir_uart.h"
#include "../../fonts/font3x5_1.h"

#define DISPLAY_UPDATE_RATE 1000

typedef enum game_state_enum {
    START_SCREEN,
    
    //sender
    CHOOSE_NUMBER_1,
    CHOOSE_OPERATOR,
    CHOOSE_NUMBER_2,
    WAIT_FOR_SEND,
    WAIT_FOR_RECEIVER,
    
    //receiver
    WAIT_FOR_QUESTION,
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
int receiver_answer = 0;
int correct_answer = 0;
int grading = 0;

char question[6] = {0};

char digit_base_10_to_char(int digit) {
    char chr = '0';
    chr += digit;
    return chr;
}

char * to_text(int num) {
    
    // we only want to deal with positive numbers in the range 0-99
    num = add_modulo(num, 0, 100);
    

    // note that we use this as a global string
    char * str = "  ";
    str[0] = ' ';
    str[1] = ' ';
    
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
    tinygl_text ("  NAVSWITCH OR WAIT");
    
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
    tinygl_text ("  BUT1 TO SEND!");
    
}

void change_to_wait_for_receiver() {
    
    game_state = WAIT_FOR_RECEIVER;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  ...");
    
}

void change_to_display_question() {
    
    game_state = DISPLAY_QUESTION;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text (question); //placeholder
}

void change_to_choose_answer () {
    
    game_state = CHOOSE_ANSWER;
    
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (to_text(receiver_answer));
    tinygl_text_speed_set (100);
    
}

void change_to_display_result() {
    
    game_state = DISPLAY_RESULT;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
}

void change_to_wait_for_question() {

    game_state = WAIT_FOR_QUESTION;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  ...");



}

int add_modulo(int number, int amount, int modulo) {
    
    int sum = (number + amount) % modulo;
    
    if (sum < 0) {
        sum += modulo;
    }
    
    return sum;
    
}

bool is_question_valid(char * _question) {
    
    return (_question[0] >= '0' && _question[0] <= '9') || _question[0] == ' '
            && (_question[1] >= '0' && _question[1] <= '9')
            && (_question[3] >= '0' && _question[3] <= '9')
            && (_question[4] >= '0' && _question[4] <= '9')
            && (_question[2] == '+' || _question[2] == 'X' || _question[2] == '-' || _question[2] == '/');

}

int calculate_correct_answer (char * _question) {
    
    int digit_1_1;
    if (_question[0] == ' ') {
        digit_1_1 = 0;
    } else {
        digit_1_1 = _question[0] - '0';
    }
    int digit_1_2 = _question[1] - '0';


    int digit_2_1;
    if (_question[3] == ' ') {
        digit_2_1 = 0;
    } else {
        digit_2_1 = _question[3] - '0';
    }
    int digit_2_2 = _question[4] - '0';

    int number_1 = digit_1_1 * 10 + digit_1_2;
    int number_2 = digit_2_1 * 10 + digit_2_2;

    if (_question[2] == '+') {
        return number_1 + number_2;
    } else if (_question[2] == 'X') {
        return number_1 * number_2;
    } else if (_question[2] == '-') {
        return number_1 - number_2;
    } else {
        return number_1 / number_2;
    }
    
}

int check_receiver_answer(int correct_answer, int receiver_answer) {
    
    //return positive integer if correct
    if (receiver_answer == correct_answer) {
        grading = 1;
    }
    
    else {
        grading = 0;
    }
    
    return grading;
    
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
        } else if (ir_uart_read_ready_p()) {
            char recv = ir_uart_getc();
            if (recv == '>') {

                if (ir_uart_write_ready_p ()) {
                    ir_uart_putc('<');
                }
 
                change_to_display_question();
            }
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
        
        int range;
        if (sender_operator == OP_SUB) {
            range = sender_number_1 + 1;
        } else {
            range = 10;
        }

        number_select( &sender_number_2, range, &to_text);
        
        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            change_to_wait_for_send();
        }
        
    } else if (game_state == WAIT_FOR_SEND) {
        
        if (button_push_event_p(BUTTON1)) {
        led_set (LED1, 1);
            if (ir_uart_write_ready_p()) {
                ir_uart_putc ('>');
            }
            
        } else if (ir_uart_read_ready_p ()) {
            
            char recv = ir_uart_getc();
            if (recv == '<') {

                question[0] = to_text(sender_number_1)[0];
                question[1] = to_text(sender_number_1)[1];
                question[2] = to_operator(sender_operator)[1];
                question[3] = to_text(sender_number_2)[0];
                question[4] = to_text(sender_number_2)[1];

                ir_uart_puts(question);

            }
        }
        
    } else if (game_state == WAIT_FOR_QUESTION) {
        int i = 0;
        
        /*NEED TO TEST - WAS QUICK IDEA TO SAVE TIME SETTING EACH EQUATION TO A CHAR - MAY OR MAY NOT WORK*/
        
        for (i ; i < 6 ; i++) // loop through characters in sent string (We know all data needed will be within 6 iterations)
        {
            char * recv = ir_uart_getc();
            question[i] = recv;
        }

        if (is_question_valid(question)) {
            change_to_display_question();
        }

    } else if (game_state == DISPLAY_QUESTION) {
        
        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            change_to_choose_answer();
        }
        
    } else if (game_state == CHOOSE_ANSWER) {
        number_select( &receiver_answer, 100, &to_text);
        
        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {

            correct_answer = calculate_correct_answer(question);

            grading = receiver_answer == correct_answer;
            if (grading == true) {
                tinygl_text ("  CORRECT!");
            }
            else if (grading == false) {
                tinygl_text ("  INCORRECT!");
            }
        }
    }
    
}


int main (void)
{
    system_init ();
    tinygl_init (DISPLAY_UPDATE_RATE);
    navswitch_init();
    ir_uart_init ();
    button_init();
    
    change_to_start_screen();
    
    task_t tasks[] =
    {
        {.func = game_loop, .period = TASK_RATE / DISPLAY_UPDATE_RATE}
    };
    
    task_schedule (tasks, ARRAY_SIZE (tasks));
    
    return 0;
    
}