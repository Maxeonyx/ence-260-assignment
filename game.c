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
    STOPWATCH, ////!!!!////
    
    //receiver
    WAIT_FOR_QUESTION,
    DISPLAY_QUESTION,
    CHOOSE_ANSWER,
    DISPLAY_RESULT,

    EASTER_EGG
} GameState;

enum operator_enum {
    OP_ADD,
    OP_MUL,
    OP_SUB,
    OP_DIV
};
/* Set initial game state to start screen */
GameState game_state = START_SCREEN;

/* Initialise all global variables */
int sender_number_1 = 0;
int sender_operator = 0;
int sender_number_2 = 0;
int receiver_answer;
int time = 300;

char * result;

char question[6] = {0};

/* Function to convert int to char */
char digit_base_10_to_char(int digit) {
    char chr = '0';
    chr += digit;
    return chr;
}

/* Function used with tinygl_text to print numbers to the screen */
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

/* Function to return operator to print when in operator selection */
char * to_operator(int operator) {
    
    switch (operator) {
        case OP_ADD: return " +";
        case OP_MUL: return " X";
        case OP_SUB: return " -";
        case OP_DIV: return " /";
    }
    
    return "NO";
}

/* Sets the game state to START_SCREEN and the start screen to print instructions to the display when called */
void change_to_start_screen() {
    
    game_state = START_SCREEN;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  NAVSWITCH OR WAIT");
    
}

/* Sets game state to CHOOSE_NUMBER_1, displays a number between 0-9 for the player to choose from in game */
void change_to_choose_num_1() {
    
    game_state = CHOOSE_NUMBER_1;
    
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (to_text(sender_number_1));
    tinygl_text_speed_set (100);
    
}
/* Sets game state to CHOOSE_NUMBER_2, does same thing as 'change_to_choose_num_1' but for the second number */
void change_to_choose_num_2() {
    
    game_state = CHOOSE_NUMBER_2;
    
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (to_text(sender_number_2));
    tinygl_text_speed_set (100);
    
}

/* Sets game state to CHOOSE_OPERATOR, allows the player to 'scroll' through operators by displaying operator options */
void change_to_choose_operator() {
    
    game_state = CHOOSE_OPERATOR;
    
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (to_operator(sender_operator));
    tinygl_text_speed_set (100);
    
}

/* Changes game state to WAIT_FOR_SEND, displays instructions on how to send challenge to second player */
void change_to_wait_for_send() {
    
    game_state = WAIT_FOR_SEND;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  BUT1 TO SEND!");
    
}

/* Changes game state to WAIT_FOR_RECIEVER, displays ... while waiting */
void change_to_wait_for_receiver() {
    
    game_state = WAIT_FOR_RECEIVER;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  ...");
    
}

/* Changes game state to DISPLAY_QUESTION, displays challenge to second player */
void change_to_display_question() {
    
    game_state = DISPLAY_QUESTION;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text (question); //placeholder
}

/* Changes game state to CHOOSE_ANSWER, displays number range 0-99 for player to 'scroll' through and choose an answer */
void change_to_choose_answer () {
    
    game_state = CHOOSE_ANSWER;
    
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (to_text(receiver_answer));
    tinygl_text_speed_set (100);
    
}

/* Changes game state to DISPLAY_RESULT, displays whether the second player got the correct answer or not */
void change_to_display_result() {
    
    game_state = DISPLAY_RESULT;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text (result);
}

/* Changes game state to WAIT_FOR_QUESTION, displays ... while waiting to recieve challenge from player 1 */
void change_to_wait_for_question() {

    game_state = WAIT_FOR_QUESTION;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("  ...");

}

/* The credits!! */
void change_to_credits() {

    game_state = EASTER_EGG;
    
    tinygl_text_speed_set (28);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text ("THANKS FOR PLAYING!!!");//" WE HOPE YOU HAD FUN! -- RYELY B-D, MAXWELL C");

}

void change_to_count_down() {
    
    game_state = STOPWATCH;
    
    tinygl_text_speed_set(100);
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (time);
    
}

int add_modulo(int number, int amount, int modulo) {
    
    int sum = (number + amount) % modulo;
    
    if (sum < 0) {
        sum += modulo;
    }
    
    return sum;
    
}

 int timing() {
    
     bool run;
     static uint16_t time;
     char str[2];
    
     if (!run) {
         time = 0;
         return time;
     }
    
     str[0] = ((time /10) % 10) + '0';
     str[1] = (time % 10) + '0';
     
     tinygl_text (str);
     
     time--;
     
}

bool decode_question(unsigned char question) {

    if (question < 100) {
        sender_operator = OP_ADD;
        sender_number_1 = question / 10;
        sender_number_2 = question % 10;
    } else if (question >= 100 && question < 200) {
        sender_operator = OP_MUL;
        sender_number_1 = (question - 100) / 10;
        sender_number_2 = (question - 100) % 10;
    } else if (question >= 200 && question < 255) {
        sender_operator = OP_SUB;

        int code = question - 200;

        int count = 0;
        int i = 0;
        for (; i <= 9; i ++) {
            int j = 0;
            for (; j <= i; j ++) {
                if (count == code) {
                    sender_number_1 = i;
                    sender_number_2 = j;
                    return false;
                }
                count ++;
            }
        }


    } else {
        return true;
    }

    return false;
}

unsigned char encode_question() {

    if (sender_operator == OP_ADD) {
        return (unsigned char) (sender_number_1 * 10 + sender_number_2);
    } else if (sender_operator = OP_MUL) {
        return (unsigned char) (100 + sender_number_1 * 10 + sender_number_2);
    } else {
        unsigned char count = 0;
        int i = 0;
        int j = 0;
        while (!(i == sender_number_1 && j == sender_number_2)) {

            j ++;
            if (j > i) {
                j = 0;
                i++;
            }

            count ++;
        }

        return count;
    }

} 

int calculate_correct_answer (char * _question) {

    if (_question[2] == '+') {
        return sender_number_1 + sender_number_2;
    } else if (_question[2] == 'X') {
        return  sender_number_1 * sender_number_2;
    } else {
        return  sender_number_1 - sender_number_2;
    }
    
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
 
                change_to_wait_for_question();
            }
        } else if (button_push_event_p(BUTTON1)) {
            
            change_to_count_down();
            
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
            
                ir_uart_putc(encode_question());
                
                change_to_count_down();
                
            }
        
        }
    } else if (game_state == STOPWATCH) {
        
        if (ir_uart_read_ready_p ()) {
            
            char answered = ir_uart_getc();
            if (answered == '<') {
                change_to_start_screen();
        
            
            } else if (time == 0) {
            
            change_to_start_screen();
            
        } else {
            
            time = timing();
            
        }
        
    
    
    } else if (game_state == WAIT_FOR_QUESTION) {
        
        if (ir_uart_read_ready_p()) {
            unsigned char recv = ir_uart_getc();
            bool easter_egg = decode_question(recv);
            if(easter_egg) {
                change_to_credits();
            } else {
                question[0] = to_text(sender_number_1)[0];
                question[1] = to_text(sender_number_1)[1];
                question[2] = to_operator(sender_operator)[1];
                question[3] = to_text(sender_number_2)[0];
                question[4] = to_text(sender_number_2)[1];
                change_to_display_question();
            }
        }

    } else if (game_state == DISPLAY_QUESTION) {

        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            change_to_choose_answer();
        }
        
    } else if (game_state == CHOOSE_ANSWER) {
        number_select( &receiver_answer, 100, &to_text);
        
        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {

            int correct_answer = calculate_correct_answer(question);

            bool grading = (receiver_answer == correct_answer);
            if (grading == true) {
                result = "  CORRECT :)";
            }
            else if (grading == false) {
                result = "  INCORRECT!";
            }
            change_to_display_result();
        }
    } else if (game_state == DISPLAY_RESULT) {

        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            change_to_start_screen();
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