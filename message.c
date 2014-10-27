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
#include <assert.h>

struct message_t * message_create () {
    struct message_t * new_message = (struct message_t*) malloc ( sizeof(struct message_t) );
    return new_message;
}

struct message_t * message_create_with_tuple ( int opcode, int content_type, struct tuple_t * tuple  ) {
    struct message_t * new_message = message_create();
    
    if ( new_message != NULL ) {
        new_message->opcode = opcode;
        new_message->c_type = content_type;
        new_message->content.tuple = tuple;
    }
    
    return new_message;
}

struct message_t * message_create_with_entry ( int opcode, int content_type, struct entry_t * entry  ) {
    
    struct message_t * new_message = message_create();
    
    if ( new_message != NULL ) {
        new_message->opcode = opcode;
        new_message->c_type = content_type;
        new_message->content.entry = entry;
    }
    
    return new_message;
}

struct message_t * message_create_with_result ( int opcode, int content_type, int result  ) {
    struct message_t * new_message = message_create();
    
    if ( new_message != NULL ) {
        new_message->opcode = opcode;
        new_message->c_type = content_type;
        new_message->content.result = result;
    }
    
    return new_message;
}

int message_size_bytes ( struct message_t * msg ) {
    return OPCODE_SIZE + C_TYPE_SIZE + message_content_size_bytes(msg);
}

int message_content_size_bytes ( struct message_t * msg ) {
    
    if ( msg == NULL) {
        puts ("message_content_size_bytes but msg is NULL");
        return -1;
        
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
        printf("Unrecognized message content type\n");
        content_size_bytes = -1;
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
        buffer[0] = (char*) calloc( 1,  RESULT_SIZE );
        int result_to_network = htonl(message->content.result);
        memcpy(buffer[0], &result_to_network, RESULT_SIZE);
        buffer_size = RESULT_SIZE;
    }
    else {
        printf("message_serialize_content : invalide C_TYPE\n");
        buffer_size=-1;
    }
    
    return buffer_size;
}


/* Converte o conteúdo de uma message_t num char*, retornando o tamanho do
 * buffer alocado para a mensagem serializada como um array de
 * bytes, ou -1 em caso de erro.
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
        return -1;
        
    //gets the memory amount needed to be alloced
    int msg_buffer_size = message_size_bytes ( msg );
    //allocs the memory
    msg_buf[0] = (char*) calloc (1, msg_buffer_size );
    
    //offset
    int offset = 0;
    
    //1. adds the opcode to the buffer
    int opcode_to_network = htons(msg->opcode);
    memcpy(msg_buf[0]+offset, &opcode_to_network, OPCODE_SIZE);
    //moves offset
    offset+=OPCODE_SIZE;
    
    //2. adds the content type code
    int ctype_to_network = htons(msg->c_type);
    memcpy(msg_buf[0]+offset, &(ctype_to_network), C_TYPE_SIZE);
    //moves the offset
    offset+=C_TYPE_SIZE;
    
    //buffer to serialize the message content
    char ** message_serialized_content = (char**) calloc(1, sizeof(char*));
    // serializes the content message
    int message_serialized_content_size = message_serialize_content ( msg, message_serialized_content);
    
    //adds the content into the buffer
    memcpy(msg_buf[0]+offset, *message_serialized_content, message_serialized_content_size);
    
    free(message_serialized_content);
    
    if ( msg_buf == NULL)
        assert(2==1);

    
    return msg_buffer_size;
}

/* Transforma uma mensagem em buffer para uma struct message_t*
 */
struct message_t *buffer_to_message(char *msg_buf, int msg_size) {
    
    // OP_CODE
    int offset = 0;
    
    struct message_t * message = (struct message_t*) malloc ( sizeof(struct message_t) );
    //gets the opcode
    int opcode_network = 0;
    memcpy(&opcode_network, msg_buf+offset, OPCODE_SIZE);
    //gets to host
    int opcode_host = ntohs(opcode_network);
    //moves offset
    offset+=OPCODE_SIZE;
    //sets it
    message->opcode = opcode_host;
    

    
    //same to c_type
    //gets the opcode
    int ctype_network = 0;
    memcpy(&ctype_network, msg_buf+offset, C_TYPE_SIZE);

    //gets to host
    int ctype_host = ntohs(ctype_network);
    //moves offset
    offset+=C_TYPE_SIZE;
    //sets it
    message->c_type = ctype_host;
        
    //sets the content
    if ( ctype_host == CT_TUPLE ) {
        message->content.tuple = tuple_deserialize(msg_buf+offset, msg_size-offset);
        if ( message->content.tuple == NULL )
            return NULL;
    }
    else if ( ctype_host == CT_ENTRY ) {
        message->content.entry = entry_deserialize(msg_buf+offset, msg_size-offset);
        if ( message->content.entry == NULL )
            return NULL;
    }
    else if ( ctype_host == CT_RESULT ) {
        
        if ( msg_size-offset == RESULT_SIZE) {
            int result_network = 0;
            memcpy(&result_network, msg_buf+offset, RESULT_SIZE);
            //gets to host
            int result_host = ntohl(result_network);
            //moves offset
            offset+=RESULT_SIZE;
            //sets it
            message->content.result = result_host;
        }
        else {
                free_message(message);
                return NULL;
        }
    }

    
    return message;
}

/* Liberta a memoria alocada na função buffer_to_message
 */
void free_message(struct message_t *message) {
    
    if ( message == NULL)
        return;
    
    if ( message->c_type == CT_TUPLE ) {
        tuple_destroy(message->content.tuple);
    }
    else if ( message->c_type == CT_ENTRY ) {
        entry_destroy(message->content.entry);
    }
    
    free(message);
}




