//
//  general_utils.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 30/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include "inet.h"
#include "general_utils.h"
#include <string.h>
#include <time.h>

int is_number (char * stringWithNumber ) {
    char *ptr;
    strtol(stringWithNumber, &ptr, 10);
    
    return strncmp(ptr, "", 1) == 0;
}

/*
 * Generates a random number
 *    Formula:
 *      rand() % N   <- To get a number between 0 - N-1
 *      Then add the result to min, giving you
 *      a random number between min - max.
 */
int get_random_number(int min, int max){
    int random_number;
    random_number = (rand() % (max - min + 1) + min);
    
    return random_number;
}


