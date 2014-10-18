//
//  message-private.h
//  SD15-Product
//
//  Created by Bruno Mata on 17/10/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#include <stdio.h>

#define OPCODE_SIZE 2
#define C_TYPE_SIZE 2
#define TUPLE_DIMENSION_SIZE 4
#define TUPLE_ELEMENTSIZE_SIZE 4
#define TIMESTAMP_SIZE 8
#define ENTRY_DIMENSION_SIZE 4
#define ENTRY_ELEMENTSIZE_SIZE 4
#define RESULT_SIZE	4


static long long swap_bytes_64(long long number)
{
    long long new_number;
    
    new_number = ((number & 0x00000000000000FF) << 56 |
                  (number & 0x000000000000FF00) << 40 |
                  (number & 0x0000000000FF0000) << 24 |
                  (number & 0x00000000FF000000) << 8  |
                  (number & 0x000000FF00000000) >> 8  |
                  (number & 0x0000FF0000000000) >> 24 |
                  (number & 0x00FF000000000000) >> 40 |
                  (number & 0xFF00000000000000) >> 56);
    
    return new_number;
}
