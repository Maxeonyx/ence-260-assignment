/**

    utils.c
    Ryely Burtenshaw-Day, Maxwell Clarke

*/

#include "utils.h"

/* Function to convert int to char */
char digit_base_10_to_char(int digit) {
    char chr = '0';
    chr += digit;
    return chr;
}


/* Function to add numbers and perform a mathematical modulus operator */
int add_modulo(int number, int amount, int modulo) {
    
    int sum = (number + amount) % modulo;
    
    if (sum < 0) {
        sum += modulo;
    }
    
    return sum;
}


/* Function to return operator to print when in operator selection */
char * to_operator(int operator) {
    
    switch (operator) {
        case OP_ADD: return " +";
        case OP_MUL: return " X";
        case OP_SUB: return " -";
    }
    
    return "NO";
}