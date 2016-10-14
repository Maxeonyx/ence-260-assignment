/**

    utils.h
    Ryely Burtenshaw-Day, Maxwell Clarke

*/


#ifndef TEAM32_UTIL_H
#define TEAM32_UTIL_H


enum operator_enum {
    OP_ADD,
    OP_MUL,
    OP_SUB
};

/* Function to convert int to char */
char digit_base_10_to_char(int digit);


/* Function to add numbers and perform a mathematical modulus operator */
int add_modulo(int number, int amount, int modulo);


/* Function to return operator to print when in operator selection */
char * to_operator(int operator);


#endif