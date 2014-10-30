//
//  general_utils.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 30/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include "inet.h"

int is_number (char * stringWithNumber ) {
    char *ptr;
    strtol(stringWithNumber, &ptr, 10);
    
    return strncmp(ptr, "", 1) == 0;
}




