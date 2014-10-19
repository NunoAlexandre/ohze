#ifndef SD15_Product_message_private_h
#define SD15_Product_message_private_h

//
//  message-private.h
//  SD15-Product
//
//  Created by Grupo SD015 on 17/10/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#include <stdio.h>
#include "tuple-private.h"
#include "tuple.h"
#include "entry-private.h"
#include "entry.h"
#include "message.h"

#define OPCODE_SIZE 2
#define C_TYPE_SIZE 2
//types of message contents
#define C_TYPE_TUPLE CT_TUPLE
#define C_TYPE_ENTRY CT_ENTRY
#define C_TYPE_RESULT CT_RESULT
// sets the bytes size of size flags
#define TUPLE_DIMENSION_SIZE 4
#define TUPLE_ELEMENTSIZE_SIZE 4
#define TIMESTAMP_SIZE 8
#define ENTRY_DIMENSION_SIZE 4
#define ENTRY_ELEMENTSIZE_SIZE 4
#define RESULT_SIZE 	4


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

int message_size_bytes (  struct message_t * msg );
int message_content_size_bytes (  struct message_t * msg );

unsigned long string_positive_number ( char* numberAsString );

#endif