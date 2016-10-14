/**
 MATHS FIGHT!!
 Written by Maxwell Clarke and Ryely Burtenshaw-Day
 */

#include "our_display.h"
#include "utils.h"

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

#define COUNTDOWN_AMOUNT 7

typedef enum game_state_enum {
    START_SCREEN,
    
    //sender
    CHOOSE_NUMBER_1,
    CHOOSE_OPERATOR,
    CHOOSE_NUMBER_2,
    WAIT_FOR_SEND,
    STOPWATCH,
    
    //receiver
    WAIT_FOR_QUESTION,
    DISPLAY_QUESTION,
    CHOOSE_ANSWER,
    DISPLAY_RESULT,

    EASTER_EGG
} GameState;

/* Set initial game state to start screen */
GameState game_state = START_SCREEN;

/* Initialise all global variables */
int sender_number_1 = 0;
int sender_operator = 0;
int sender_number_2 = 0;
int receiver_answer;
int timer = COUNTDOWN_AMOUNT * DISPLAY_UPDATE_RATE;

char * result;

char question[6] = {0};


void reset_answer(void) {
    receiver_answer = 0;
}

void reset_numbers(void) {
    sender_number_1 = 0;
    sender_operator = 0;
    sender_number_2 = 0;
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


/*
    All the change_to_<whatever> functions are functions to change between game states. They perform some setup before the given state.
*/

/* Sets the game state to START_SCREEN and the start screen to print instructions to the display when called */
void change_to_start_screen(void) {

    reset_numbers();
    reset_answer();
    
    game_state = START_SCREEN;
    set_scroll_text();
    tinygl_text ("  NAVSWITCH OR WAIT");
    
}

/* Sets game state to CHOOSE_NUMBER_1, displays a number between 0-9 for the player to choose from in game */
void change_to_choose_num_1(void) {
    
    game_state = CHOOSE_NUMBER_1;
    set_static_text ();
    tinygl_text (to_text(sender_number_1));
    
}
/* Sets game state to CHOOSE_NUMBER_2, does same thing as 'change_to_choose_num_1' but for the second number */
void change_to_choose_num_2(void) {
    
    game_state = CHOOSE_NUMBER_2;
    set_static_text ();
    tinygl_text (to_text(sender_number_2));
    
}

/* Sets game state to CHOOSE_OPERATOR, allows the player to 'scroll' through operators by displaying operator options */
void change_to_choose_operator(void) {
    
    game_state = CHOOSE_OPERATOR;
    set_static_text ();
    tinygl_text (to_operator(sender_operator));
    
}

/* Changes game state to WAIT_FOR_SEND, displays instructions on how to send challenge to second player */
void change_to_wait_for_send(void) {
    
    game_state = WAIT_FOR_SEND;
    set_scroll_text();
    tinygl_text ("  BUT1 TO SEND!");
    
}

/* Changes game state to DISPLAY_QUESTION, displays challenge to second player */
void change_to_display_question(void) {
    
    game_state = DISPLAY_QUESTION;
    set_scroll_text();
    tinygl_text (question); //placeholder
}

/* Changes game state to CHOOSE_ANSWER, displays number range 0-99 for player to 'scroll' through and choose an answer */
void change_to_choose_answer (void) {
    
    game_state = CHOOSE_ANSWER;
    set_static_text ();
    tinygl_text (to_text(receiver_answer));
    
}

/* Changes game state to DISPLAY_RESULT, displays whether the second player got the correct answer or not */
void change_to_display_result(void) {
    
    game_state = DISPLAY_RESULT;
    set_scroll_text();
    tinygl_text (result);
}

/* Changes game state to WAIT_FOR_QUESTION, displays ... while waiting to recieve challenge from player 1 */
void change_to_wait_for_question(void) {

    game_state = WAIT_FOR_QUESTION;
    set_scroll_text();
    tinygl_text ("  ...");

}

/* The credits!! */
void change_to_credits(void) {

    game_state = EASTER_EGG;
    set_scroll_text();
    tinygl_text ("THANKS FOR PLAYING!!!");//" WE HOPE YOU HAD FUN! -- RYELY B-D, MAXWELL C");

}

void change_to_count_down(void) {
    
    timer = COUNTDOWN_AMOUNT * DISPLAY_UPDATE_RATE;
    
    game_state = STOPWATCH;
    set_static_text ();
    tinygl_text (to_text(timer/DISPLAY_UPDATE_RATE));
    
}


/*
    Encodes all the information about a question into one character.
    The mapping is as follows:

        0-99   : Addition
         - e.g.  12 is 1 + 2

        100-199: Multiplication
         - e.g. 112 is 1 * 2

        200-254: Subtraction
         - This follows a different mapping. Examples:

            200: 0 - 0

            201: 1 - 0
            202: 1 - 1

            203: 2 - 0
            204: 2 - 1
            205: 2 - 2
            ...

            There are only 55 possible subtraction questions because we do not
            allow questions with negative answers.

*/
unsigned char encode_question(void) {

    if (sender_operator == OP_ADD) {
        return (unsigned char) (sender_number_1 * 10 + sender_number_2);
    } else if (sender_operator == OP_MUL) {
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

        return count + 200;
    }

}

/*
    Decodes the question information out of the received character.
*/
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


/*
    All the run_<whatever> functions are the components of the game loop.
    Each corresponds to a particular game state.
*/


void run_start_screen(void) {
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {

        change_to_choose_num_1();

    } else if (ir_uart_read_ready_p()) {

        char recv = ir_uart_getc();
        if (recv == '>') {

            if (ir_uart_write_ready_p ()) {
                ir_uart_putc('<');

                change_to_wait_for_question();
            }
        }

    }
}

void run_choose_number_1(void) {
    number_select( &sender_number_1, 10, &to_text);
    
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
        change_to_choose_operator();
    }
}

void run_choose_operator(void) {
    number_select( &sender_operator, 3, &to_operator);
    
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
        change_to_choose_num_2();
    }
}

void run_choose_number_2(void) {
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
}

void run_wait_for_send(void) {
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
}

void run_stopwatch(void) {
    if (timer % DISPLAY_UPDATE_RATE == 0) {
        tinygl_text (to_text(timer/DISPLAY_UPDATE_RATE));
    }

    timer -= 1;
    
    if (ir_uart_read_ready_p ()) {
        
        char answered = ir_uart_getc();
        if (answered == '(') {

            change_to_start_screen();
        
        }
        
    } else if (timer < 10) {

        if (ir_uart_write_ready_p()) {

            ir_uart_putc(')');
            change_to_start_screen();

        }
    
    }
}

void run_wait_for_question(void) {
    if (ir_uart_read_ready_p()) {
        unsigned char recv = ir_uart_getc();
        bool easter_egg = decode_question(recv);
        if(easter_egg) {
            change_to_credits();
        } else {
            question[0] = to_text(sender_number_1)[0];
            question[1] = to_text(sender_number_1)[1];
            question[2] = to_operator(sender_operator)[1];
            question[3] = to_text(sender_number_2)[1];
            question[4] = to_text(sender_number_2)[0];
            change_to_display_question();
        }
    }
}

void run_display_question(void) {
    if (ir_uart_read_ready_p()) {

        char recv = ir_uart_getc();
        if (recv == ')') {
            result = "  ...TIME UP!";
            change_to_display_result();
        }

    }

    if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
        change_to_choose_answer();
    }
}

void run_choose_answer(void) {
    number_select( &receiver_answer, 100, &to_text);

    if (ir_uart_read_ready_p()) {

        char recv = ir_uart_getc();
        if (recv == ')') {
            result = "  ...TIME UP!";
            change_to_display_result();
        }

    }
    
    if (navswitch_push_event_p(NAVSWITCH_PUSH)) {

        int correct_answer = calculate_correct_answer(question);

        bool grading = (receiver_answer == correct_answer);

        if (grading == true) {
            result = "  CORRECT :)";
        }
        else if (grading == false) {
            result = "  INCORRECT!";
        }

        if (ir_uart_write_ready_p()) {

            ir_uart_putc('(');

            change_to_display_result();
        }
    }
}

void run_display_result(void) {
    if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
        change_to_start_screen();
    }
}

void run_easter_egg(void) {
    // Do nothing. Trapped!
}


static void game_loop (__unused__ void *data) {
    
    tinygl_update ();
    
    button_update();
    navswitch_update();
    
    switch(game_state) {

        case START_SCREEN:
            run_start_screen();
            break;
        
        case CHOOSE_NUMBER_1:
            run_choose_number_1();
            break;
            
        case CHOOSE_OPERATOR:
            run_choose_operator();
            break;
            
        case CHOOSE_NUMBER_2:
            run_choose_number_2();
            break;
            
        case WAIT_FOR_SEND:
            run_wait_for_send();
            break;
            
        case STOPWATCH:
            run_stopwatch();
            break;
            
        case WAIT_FOR_QUESTION:
            run_wait_for_question();
            break;

        case DISPLAY_QUESTION:
            run_display_question();
            break;
            
        case CHOOSE_ANSWER:
            run_choose_answer();
            break;
            
        case DISPLAY_RESULT:
            run_display_result();
            break;

        case EASTER_EGG:
            run_easter_egg();
            break;

    } 
    
}


int main (void)
{
    system_init ();
    tinygl_init (DISPLAY_UPDATE_RATE);
    navswitch_init();
    ir_uart_init ();
    button_init();
    
    tinygl_font_set (&font3x5_1);
    
    change_to_start_screen();
    
    task_t tasks[] =
    {
        {.func = game_loop, .period = TASK_RATE / DISPLAY_UPDATE_RATE}
    };
    
    task_schedule (tasks, ARRAY_SIZE (tasks));
    
    return 0;
    
}
