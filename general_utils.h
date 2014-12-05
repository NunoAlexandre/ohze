//
//  general_utils.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 30/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//
#ifndef SD15_Product_general_utils_h
#define SD15_Product_general_utils_h

#include <unistd.h>

#define TASK_FAILED -1
#define TASK_SUCCEEDED 0
#define YES 1
#define NO  0




#define CLIENT_RECEIVE_TUPLES 1
#define CLIENT_PROCEED 0

int is_number (char * stringWithNumber );

/*
 * Generates a random number
 *    Formula:
 *      rand() % N   <- To get a number between 0 - N-1
 *      Then add the result to min, giving you
 *      a random number between min - max.
 */
int get_random_number(int min, int max);

#endif
