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
#define TOKEN_STRING_SIZE   4


long long swap_bytes_64(long long number);

/*
 * Creates a message
 */
struct message_t * message_create ();

/*
 * Creates a message with the given attributes.
 */
struct message_t * message_create_with ( int opcode, int content_type, void * content  );

/*
 * Creates an array of msg_num messages
 */
struct message_t ** message_create_set ( int msg_num );

/*
 * Frees num messages from message_set
 */
void free_message_set(struct message_t ** message_set, int num);

/*
 * Returns the tuple_t from the message or NULL if msg is NULL.
 * Assumes the invoker knows what he is doing, ie., that
 * the message contains a tuple as its content.
 */
struct tuple_t * tuple_from_message(struct message_t * msg );

/*
 * Returns the size of the given message in bytes.
 */
int message_size_bytes (  struct message_t * msg );

/*
 * Returns the size in bytes of the message's content.
 */
int message_content_size_bytes (  struct message_t * msg );

/*
 * More flexible free_message function that gets the option free_content (YES/NO).
 */
void free_message2(struct  message_t * message, int free_content);

/*
 * Returns a message_t * built from the command string.
 * Assumes the command is valid.
 */
struct message_t * command_to_message (const char * command);


int find_ctype (const char * input );
/*
 * Given an opcode returns/assigns a proper c_type.
 */
int assign_ctype (int opcode, int mode);

/*
 * Returns the opcode from the input, assuming its valid.
 */
int find_opcode_as_string(const char *input);

int find_opcode_as_int(const char * input);

/*
 * Prints a given message.
 */
void message_print ( struct message_t * msg );

/*
 * Checks if response_msg means success upon the request_msg. YES or NO
 */
int response_with_success ( struct message_t *  request_msg, struct message_t * response_msg);

/*
 * Checks if the msg's opcode is valid/exists. YES or NO
 */
int message_valid_opcode ( struct  message_t * msg );

/*
 * Checks if the operation of the msg is a getter.
 */
int message_opcode_getter ( struct  message_t * msg);

/*
 * Creates an error message
 */
struct message_t * message_of_error();

/*
 * Verifies if message has error code or is NULL
 */
int message_error (struct message_t* tested_msg);
/*
 * Verifies if message is a report message
 */
int message_report (struct message_t* msg); 

int message_update_request (struct message_t* msg);

/*
  * Check if message is writer
  */
int message_is_writer(struct message_t *msg);

/*
  * Check if message is reader
  */
int message_is_reader(struct message_t *msg);

/*
 * Check is message has opcode setter
 */
int message_opcode_setter(struct message_t * msg );

/*
* Checks if message has opcode taker, ie, a getter that takes on reading.
*/
int message_opcode_taker( struct message_t * msg );

/*
 * Check is message has opcode getter
 */
int message_opcode_getter(struct  message_t * msg);

/*
 * Check is message has opcode size
 */
int message_opcode_size(struct message_t * msg );

/*
 * Check is message has opcode report
 */
int message_opcode_report (struct message_t * msg);

/*
 *  Serializes content of a given message_t
 */
int message_serialize_content ( struct message_t * message, char ** buffer );

char * message_to_string ( struct message_t * msg );

/*
 * Returns the size (bytes) of a given token
 */
int token_size_bytes (char* token);

/*
 * Returns the size in bytes of a token as being serialized.
 * This means the return value is TOKEN_STRING_SIZE + token_size_bytes(token).
 */
int token_as_serialized_size(char* token);

/*
 * Serializes a given token
 */
int token_serialize(char* token, char **buffer);

/*
 * Deserializes a given token
 */
char* token_deserialize(char* buffer, int size);

unsigned long string_positive_number ( char* numberAsString );

#endif