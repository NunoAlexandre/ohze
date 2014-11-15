//
//  message.c
//  SD15-Product
//
//  Created by Grupo SD015 on 07/10/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"
#include "message-private.h"
#include "tuple.h"
#include "tuple-private.h"
#include "entry-private.h"
#include "list-private.h"
#include <assert.h>
#include "general_utils.h"

struct message_t * message_create () {
    struct message_t * new_message = (struct message_t*) malloc ( sizeof(struct message_t) );
    return new_message;
}

struct message_t * message_create_with ( int opcode, int content_type, void * element  ) {
    
    struct message_t * new_message = message_create();
    
    //sets the content of the message
    if ( new_message != NULL ) {
        new_message->opcode = opcode;
        new_message->c_type = content_type;
        //depending on the content_type sets the content
        switch (content_type) {
            case CT_ENTRY:
                new_message->content.entry = element;
                break;
            case CT_TUPLE:
                new_message->content.tuple = element;
                break;
            case CT_RESULT:
                new_message->content.result = * ((int *) element);
                break;
            default:
                break;
        }
    }
    
    return new_message;
}
struct message_t ** message_create_set ( int msg_num ) {
    struct message_t ** msg_set =  malloc (  msg_num *  sizeof(struct message_t *) );
    return msg_set;
}

void free_message_set(struct message_t ** message_set, int num) {
    if ( message_set != NULL ) {
        int i =0;
        for ( i= 0; i<num; i++) {
            free_message2(message_set[i], NO );
        }
    }
}
struct tuple_t * tuple_from_message(struct message_t * msg ) {
    return msg == NULL ? NULL : msg->content.tuple;
} 

int message_size_bytes ( struct message_t * msg ) {
    return OPCODE_SIZE + C_TYPE_SIZE + message_content_size_bytes(msg);
}

int message_content_size_bytes ( struct message_t * msg ) {
    
    if ( msg == NULL) {
        puts ("message_content_size_bytes but msg is NULL");
        return TASK_FAILED;
        
    }
    
    //the content size in bytes
    int content_size_bytes = 0;
    
    if ( msg->c_type == CT_TUPLE ) {
        content_size_bytes = tuple_size_bytes(msg->content.tuple);
    }
    else if ( msg->c_type == CT_ENTRY ) {
        content_size_bytes = entry_size_bytes(msg->content.entry);
    }
    else if ( msg->c_type == CT_RESULT ) {
        content_size_bytes = RESULT_SIZE;
    }
    else {
        printf("Unrecognized message content type : value is %d\n", msg->c_type);

        content_size_bytes = TASK_FAILED;
    }
    
    return content_size_bytes;
}

int message_serialize_content ( struct message_t * message, char ** buffer ) {
    
    int buffer_size = 0;
    
    if ( message->c_type == CT_TUPLE ) {
        buffer_size = tuple_serialize(message->content.tuple, buffer);
    }
    else  if ( message->c_type == CT_ENTRY ) {
        buffer_size = entry_serialize(message->content.entry, buffer);
    }
    else if ( message->c_type == CT_RESULT ) {
        buffer[0] = (char*) malloc(RESULT_SIZE );
        int result_to_network = htonl(message->content.result);
        memcpy(buffer[0], &result_to_network, RESULT_SIZE);
        buffer_size = RESULT_SIZE;
    }
    else {
        printf("message_serialize_content : invalide C_TYPE\n");
        buffer_size=TASK_FAILED;
    }
    
    return buffer_size;
}


/* Converte o conteúdo de uma message_t num char*, retornando o tamanho do
 * buffer alocado para a mensagem serializada como um array de
 * bytes, ou TASK_FAILED em caso de erro.
 *
 * A mensagem serializada deve ter o seguinte formato:
 *
 * OPCODE		C_TYPE
 * [2 bytes]	[2 bytes]
 *
 * a partir daí, o formato difere para cada c_type:
 *
 * ct_type	dados
 * TUPLE	DIMENSION	ELEMENTSIZE	ELEMENTDATA	...
 *		[4 bytes]	 [4 bytes]	[ES bytes]
 * ENTRY	TIMESTAMP 	DIMENSION	ELEMENTSIZE ELEMENTDATA
 *		          [8 bytes] 	[4 bytes]	 [4 bytes]	[ES bytes]	...
 * RESULT	RESULT
 *		[4 bytes]
 *
 */
int message_to_buffer(struct message_t *msg, char **msg_buf) {
    
    if ( msg == NULL )
        return TASK_FAILED;
    
    //gets the memory amount needed to be alloced
    int msg_buffer_size = message_size_bytes ( msg );
    //allocs the memory
    *msg_buf = (char*) malloc( msg_buffer_size );

    //offset
    int offset = 0;
    
    //1. adds the opcode to the buffer
    int opcode_to_network = htons(msg->opcode);

    memcpy(msg_buf[0]+offset, &opcode_to_network, OPCODE_SIZE);

    //moves offset
    offset+=OPCODE_SIZE;
    
    //2. adds the content type code
    int ctype_to_network = htons(msg->c_type);

    memcpy(msg_buf[0]+offset, &ctype_to_network, C_TYPE_SIZE);

    //moves the offset
    offset+=C_TYPE_SIZE;
    
    //buffer to serialize the message content
    char * message_serialized_content = NULL;
    // serializes the content message
    int message_serialized_content_size = message_serialize_content ( msg, &message_serialized_content);
    
    if ( message_serialized_content_size == TASK_FAILED || message_serialized_content == NULL) {
        return TASK_FAILED;
    }

    //adds the content into the buffer
    memcpy(msg_buf[0]+offset, message_serialized_content, message_serialized_content_size);

    //frees it
    free(message_serialized_content);

    
    return msg_buffer_size;
}

/* Transforma uma mensagem em buffer para uma struct message_t*
 */
struct message_t *buffer_to_message(char *msg_buf, int msg_size) {
    
    // OP_CODE
    int offset = 0;
    
    //gets the opcode
    int opcode_network = 0;
    memcpy(&opcode_network, msg_buf+offset, OPCODE_SIZE);
    //gets to host
    int opcode_host = ntohs(opcode_network);
    //moves offset
    offset+=OPCODE_SIZE;
    //sets it
    
    //same to c_type
    //gets the opcode
    int ctype_network = 0;
    memcpy(&ctype_network, msg_buf+offset, C_TYPE_SIZE);
    
    //gets to host
    int ctype_host = ntohs(ctype_network);
    //moves offset
    offset+=C_TYPE_SIZE;
    //sets it
    void * message_content = NULL;
    
    switch (ctype_host) {
        case CT_TUPLE:
            message_content = tuple_deserialize(msg_buf+offset, msg_size-offset);
            break;
        case CT_ENTRY:
            message_content = entry_deserialize(msg_buf+offset, msg_size-offset);
            break;
        case CT_RESULT:
        {
            int result_network = 0;
            memcpy(&result_network, msg_buf+offset, RESULT_SIZE);
            //gets to host
            int result_host = ntohl(result_network);
            message_content = &result_host;
            break;
        }
        default:
            break;
    }
    
    if ( message_content == NULL ) {
        return NULL;
    }
    
    //finally creates the message with all its components
    struct message_t * message = message_create_with(opcode_host, ctype_host, message_content);
    
    return message != NULL ? message : NULL;
}

/* Liberta a memoria alocada na função buffer_to_message
 */
void free_message(struct message_t *message) {
    free_message2(message, YES);    
}

void free_message2(struct  message_t * message, int free_content) {
    if ( message == NULL)
        return;

    if ( free_content ) { 
         if ( message->c_type == CT_TUPLE ) {
             tuple_destroy(message->content.tuple);
         }
        else if ( message->c_type == CT_ENTRY ) {
            entry_destroy(message->content.entry);
        }
    } 
    
    free(message);
}

struct message_t * message_of_error () {
    int taskFailedFlag = TASK_FAILED;
    return message_create_with(OC_ERROR, CT_RESULT, &taskFailedFlag);
}

/*
 *  Verifies if message has error code or is NULL
 */
int message_error (struct message_t* tested_msg){
    return tested_msg->opcode == OC_ERROR;
}

/*
 * função que compara duas mensagens - response with success --- DECLARAR EM MESSAGE-PRIVATE.H
 */
int response_with_success ( struct message_t* request_msg, struct message_t* received_msg){
    
    int opcode_req = request_msg->opcode;
    int opcode_resp = received_msg->opcode;
 
    if (message_error(received_msg)) {
        puts(" (received message is an error message)");
        return NO;
    }
    else if ( opcode_resp != (opcode_req+1) ) {
        puts(" (received message has not the expected opcode)");
        return NO;
    }
    
    return YES;
}

/*
 *  Find opcode form user input
 */

int find_opcode(const char *input ){
    
    char * input_dup = strdup(input);
    char *user_task = strdup(strtok(input_dup," "));
    
    char *out = "out";
    char *in = "in";
    char *in_all = "in_all";
    char *copy = "copy";
    char *copy_all = "copy_all";
    char *size = "size\n";
    char *quit = "quit\n";
    
    if (strcasecmp(user_task, out) == 0) {
        free(user_task);
        free(input_dup);
        return OC_OUT;
    }
    else if (strcasecmp(user_task, in) == 0) {
        free(user_task);
        free(input_dup);
        return OC_IN;
    }
    else  if (strcasecmp(user_task, in_all) == 0) {
        free(user_task);
        free(input_dup);
        return OC_IN_ALL;
    }
    else  if (strcasecmp(user_task, copy) == 0) {
        free(user_task);
        free(input_dup);
        return OC_COPY;
    }
    else if (strcasecmp(user_task, copy_all) == 0) {
        free(user_task);
        free(input_dup);
        return OC_COPY_ALL;
    }
    else  if (strcasecmp(user_task, size) == 0) {
        free(user_task);
        free(input_dup);
        return OC_SIZE;
    }
    else if (strcasecmp(user_task, quit) == 0 ) {
        free(user_task);
        free(input_dup);
        return OC_QUIT;
    }
    else {
        //if none was it...
        free(user_task);
        free(input_dup);
        return OC_DOESNT_EXIST;
    }
}

/*
 *  Assigns CTCODE according with OPCODE
 */
int assign_ctype (int opcode){
    
    int ctcode = TASK_FAILED;
    
    switch (opcode) {
            
            /* all operations that envolves finding elements from table */
        case OC_IN:
        case OC_COPY:
        case OC_IN_ALL:
        case OC_COPY_ALL:
        case OC_OUT:
        {
            ctcode = CT_TUPLE;
            break;
        }
            
            /* operation that envolves returning a value */
        case OC_SIZE:
        {
            ctcode = CT_RESULT;
            break;
        }
            
            
        default:
            ctcode = TASK_FAILED;
            break;
    }
    
    return ctcode;
}

struct message_t * command_to_message (const char * command) {
    //get opcode
    int opcode = find_opcode(command);
    
    //get ctype
    int ctype = assign_ctype(opcode);

    void * message_content = NULL;
    
    if ( ctype == CT_TUPLE ) {
        message_content = create_tuple_from_input (command);
    }
    else if ( ctype == CT_RESULT ) {
        int resultValue = 0;
        message_content = &resultValue;
    }
    //create and return message
    struct message_t * message = message_create_with(opcode, ctype, message_content);

    return message;
}

void message_print ( struct message_t * msg ) {
    if ( msg == NULL )
        printf(" [null message] ");
    else {
        if ( msg->c_type == CT_TUPLE ) {
            printf(" [%hd , %hd , ", msg->opcode, msg->c_type );
            tuple_print(msg->content.tuple);
            printf(" ] ");
        }
        else if ( msg->c_type == CT_RESULT ) {
            printf(" [%hd , %hd , %d ] ", msg->opcode, msg->c_type, msg->content.result );
        }
    }
}

int message_opcode_setter (struct message_t * msg ) {
    return msg != NULL && msg->opcode == OC_OUT;
}
int message_opcode_getter ( struct  message_t * msg) {
    return msg != NULL && (msg->opcode == OC_IN || msg->opcode == OC_IN_ALL 
            || msg->opcode == OC_COPY || msg->opcode == OC_COPY_ALL );
}

int message_opcode_size (struct message_t * msg ) {
    return msg != NULL && msg->opcode == OC_SIZE;
}

int message_valid_opcode ( struct  message_t * msg ) {
    return msg != NULL && 
        ( message_opcode_setter(msg) || message_opcode_getter(msg) || message_opcode_size(msg) );
}