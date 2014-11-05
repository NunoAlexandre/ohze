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

//operation codes
#define OC_ERROR -99
#define OC_QUIT     111
#define OC_DOESNT_EXIST 404

#define BUFFER_INTEGER_SIZE 4
#define OPCODE_SIZE 2
#define C_TYPE_SIZE 2

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

struct message_t * message_create_with ( int opcode, int content_type, void * element  );
int message_size_bytes (  struct message_t * msg );
int message_content_size_bytes (  struct message_t * msg );
unsigned long string_positive_number ( char* numberAsString );
void free_message2(struct  message_t * message, int free_content);
struct message_t * command_to_message (const char * command);

int assign_ctype (int opcode);
    
int find_opcode(const char *input_dup);


void message_print ( struct message_t * msg );
/*
 * função que compara duas mensagens - response with success --- DECLARAR EM MESSAGE-PRIVATE.H
 */
int response_with_success ( struct message_t *  request_msg, struct message_t * received_msg);

/*
 *  Verifies if message has error code or is NULL
 */
int message_error (struct message_t* tested_msg);

#endif