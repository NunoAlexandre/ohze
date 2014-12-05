//
//  message.c
//  SD15-Product
//
//  Created by Grupo SD015 on 07/10/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
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
#include "inet.h"

/*
 * Creates a message_t
 */
struct message_t * message_create () {
    struct message_t * new_message = (struct message_t*) malloc ( sizeof(struct message_t) );
    return new_message;
}

/*
 * Creates a message with the given attributes.
 * (Atualizado para Projeto 5)
 */
struct message_t * message_create_with ( int opcode, int content_type, void * element) {
    
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
                // * (Atualizado para Projeto 5) - TOKEN
            case CT_SFAILURE:
            case CT_SRUNNING:
            case CT_INVCMD:
                new_message->content.token = element;
                break;
                
            default:
                break;
        }
    }
    
    return new_message;
}

/*
 * Creates an array of msg_num messages
 */
struct message_t ** message_create_set ( int msg_num ) {
    struct message_t ** msg_set = malloc (msg_num * sizeof(struct message_t *) );
    return msg_set;
}

/*
 * Frees num messages from message_set
 */
void free_message_set(struct message_t ** message_set, int num) {
    if ( message_set != NULL ) {
        int i =0;
        for ( i= 0; i<num; i++) {
            free_message2(message_set[i], NO );
        }
    }
}

/*
 * Returns the tuple_t from the message or NULL if msg is NULL.
 * Assumes the invoker knows what he is doing, ie., that
 * the message contains a tuple as its content.
 */
struct tuple_t * tuple_from_message(struct message_t * msg ) {
    return msg == NULL ? NULL : msg->content.tuple;
}

/*
 * Returns the size of the given message in bytes.
 */
int message_size_bytes ( struct message_t * msg ) {
    return OPCODE_SIZE + C_TYPE_SIZE + message_content_size_bytes(msg);
}

/*
 * Returns the size in bytes of the message's content.
 * (Atualizado para Projeto 5)
 */
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
    else if (msg->c_type == CT_SFAILURE || msg->c_type == CT_SRUNNING || msg->c_type == CT_INVCMD){
        content_size_bytes = token_as_serialized_size(msg->content.token);
    }
    
    else {
        printf("Unrecognized message content type : value is %d\n", msg->c_type);
        
        content_size_bytes = TASK_FAILED;
    }
    
    return content_size_bytes;
}

/*
 *  Serializes content of a given message_t
 * (Atualizado para Projeto 5)(CT_SFAILURE || CT_SRUNNING)
 */
int message_serialize_content ( struct message_t * message, char ** buffer ) {
    
    int buffer_size = 0;
    
    if ( message->c_type == CT_TUPLE ) {
        buffer_size = tuple_serialize(message->content.tuple, buffer);
    }
    
    else if ( message->c_type == CT_ENTRY ) {
        buffer_size = entry_serialize(message->content.entry, buffer);
    }
    else if ( message->c_type == CT_RESULT ) {
        buffer[0] = (char*) malloc(RESULT_SIZE );
        int result_to_network = htonl(message->content.result);
        memcpy(buffer[0], &result_to_network, RESULT_SIZE);
        buffer_size = RESULT_SIZE;
    }
    else if (message->c_type == CT_SFAILURE
             || message->c_type == CT_SRUNNING
             || message->c_type == CT_INVCMD )
    {
        buffer_size = token_serialize(message->content.token, buffer);
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
 * OPCODE C_TYPE
 * [2 bytes] [2 bytes]
 *
 *  a partir daí, o formato difere para cada c_type:
 *
 *  ct_type dados
 *  TUPLE DIMENSION ELEMENTSIZE ELEMENTDATA ...
 *          [4 bytes] [4 bytes] [ES bytes]
 *
 * ENTRY TIMESTAMP DIMENSION ELEMENTSIZE ELEMENTDATA
 *     [8 bytes] [4 bytes] [4 bytes] [ES bytes] ...
 *
 * RESULT RESULT
 *          [4 bytes]
 *
 * TOKEN    DIMENSION   TOKENDATA
 *          [4 BYTES]   [TD BYTES]
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

/*
 * Transforma uma mensagem em buffer para uma struct message_t*
 * (Atualizado para Projeto 5)
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
        case CT_SRUNNING:
        case CT_SFAILURE:
        case CT_INVCMD:
        {
            message_content = token_deserialize (msg_buf + offset, msg_size-offset);
            break;
        }
            
        default:
            break;
    }
    
    if ( message_content == NULL ) {
        printf("message_content == NULL\n");
        return NULL;
    }
    
    //finally creates the message with all its components
    struct message_t * message = message_create_with(opcode_host, ctype_host, message_content);
    
    return message != NULL ? message : NULL;
}

/*
 *  Liberta a memoria alocada na função buffer_to_message
 */
void free_message(struct message_t *message) {
    free_message2(message, YES);
}

/*
 * More flexible free_message function that gets the option free_content (YES/NO).
 * (Atualizado para Projeto 5)
 */
void free_message2(struct message_t * message, int free_content) {
    if ( message == NULL)
        return;
    
    if ( free_content ) {
        if ( message->c_type == CT_TUPLE ) {
            tuple_destroy(message->content.tuple);
        }
        else if ( message->c_type == CT_ENTRY ) {
            entry_destroy(message->content.entry);
        }
        
        // * (Atualizado para Projeto 5)
        else if (message->c_type == CT_SFAILURE || message->c_type == CT_SRUNNING){
            free(message->content.token);
        }
    }
    free(message);
}

/*
 * Creates an error message
 */
struct message_t * message_of_error () {
    int taskFailedFlag = TASK_FAILED;
    return message_create_with(OC_ERROR, CT_RESULT, &taskFailedFlag);
}

/*
 *  Verifies if message has error code or is NULL
 */
int message_error (struct message_t* msg){
    return msg->opcode == OC_ERROR;
}

/*
 * Verifies if message is a report message
 */
int message_report (struct message_t* msg){
    return msg->opcode == OC_REPORT;
}


/*
 * Checks if response_msg means success upon the request_msg. YES or NO
 */
int response_with_success ( struct message_t* request_msg, struct message_t* response_msg){
    
    int opcode_req = request_msg->opcode;
    int opcode_resp = response_msg->opcode;
    
    if (message_error(response_msg)) {
        puts(" (received message is an error message)");
        return NO;
    }
    else if (message_report(response_msg)) {
        puts(" (received message is an report message)");
        return NO;
    }
    else if ( opcode_resp != (opcode_req+1) ) {
        puts(" (received message has not the expected opcode)");
        return NO;
    }
    
    return YES;
}

/*
 *  Find opcode form user input
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
    
    else if (strcasecmp(user_task, in_all) == 0) {
        free(user_task);
        free(input_dup);
        return OC_IN_ALL;
    }
    
    else if (strcasecmp(user_task, copy) == 0) {
        free(user_task);
        free(input_dup);
        return OC_COPY;
    }
    
    else if (strcasecmp(user_task, copy_all) == 0) {
        free(user_task);
        free(input_dup);
        return OC_COPY_ALL;
    }
    
    else if (strcasecmp(user_task, size) == 0) {
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
 *  Assigns CTCODE according with OPCODE
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

/*
 * Returns a message_t * built from the command string.
 * Assumes the command is valid.
 */
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

/*
 * Prints a given message.
 * (Atualizado para Projeto 5)
 */
void message_print ( struct message_t * msg ) {
    if ( msg == NULL )
        printf(" [null message] ");
    else {
        if ( msg->c_type == CT_TUPLE ) {
            printf(" [%hd , %hd , ", msg->opcode, msg->c_type );
            tuple_print(msg->content.tuple);
            printf(" ] ");
        }
        
        // * (Atualizado para Projeto 5)
        else if (msg->c_type == CT_SFAILURE || msg->c_type == CT_SRUNNING || msg->c_type == CT_INVCMD ){
            printf (" [ %hd , %hd , %s ]", msg->opcode, msg->c_type, msg->content.token);
        }
        
        else if ( msg->c_type == CT_RESULT ) {
            printf(" [%hd , %hd , %d ] ", msg->opcode, msg->c_type, msg->content.result );
        }
    }
}

/*
 * Check if message is writer (Criado para Projeto 5)
 */
int message_is_writer(struct message_t *msg) {
    return message_opcode_getter(msg) || message_opcode_setter(msg);
}

/*
 * Check if message is reader (Criado para Projeto 5)
 */
int message_is_reader(struct message_t *msg) {
    return ! message_is_writer(msg);
}

/*
 * Check if message has opcode report (Criado para Projeto 5)
 */
int message_opcode_report (struct message_t * msg){
    return msg != NULL && msg->opcode == OC_REPORT;
}

/*
 * Check is message has opcode setter
 */
int message_opcode_setter (struct message_t * msg ) {
    return msg != NULL && msg->opcode == OC_OUT;
}

/*
 * Check is message has opcode getter
 */
int message_opcode_getter ( struct message_t * msg) {
    return msg != NULL && (msg->opcode == OC_IN || msg->opcode == OC_IN_ALL
                           || msg->opcode == OC_COPY || msg->opcode == OC_COPY_ALL );
}

/*
 * Check is message has opcode size
 */
int message_opcode_size (struct message_t * msg ) {
    return msg != NULL && msg->opcode == OC_SIZE;
}

/*
 * Checks if the msg's opcode is valid/exists. YES or NO
 * (Atualizado para projeto 5)
 */
int message_valid_opcode ( struct message_t * msg ) {
    return msg != NULL &&
    ( message_opcode_setter(msg) || message_opcode_getter(msg) || message_opcode_size(msg) || message_opcode_report(msg) );
}

/*
 * Returns the size (bytes) of a given token (Criado para Projeto 5)
 */
int token_size_bytes (char* token) {
    return token == NULL ? 0 : (int) strlen(token);
}
int token_as_serialized_size(char* token) {
    return TOKEN_STRING_SIZE + token_size_bytes(token);
    
}
/*
 * Serializes a given token
 */
int token_serialize(char* token, char **buffer) {
    
    if ( token == NULL)
        return TASK_FAILED;
    
    //1. Alocar memória para token (n bytes para alocar)
    int size_of_token = token_size_bytes(token);
    int serialized_token_size = token_as_serialized_size(token);
    
     //aloca tamanho do token + TOKEN_STRING_SIZE
    *buffer = malloc(serialized_token_size);
    //para gerir preenchimento do buffer
    int offset = 0;
    
    //2. Insere dimensão do TOKEN
    //2.1 Tamanho do token para formato rede
    int size_of_token_htonl = htonl(size_of_token); //
    //2.2 Copia o tamanho do TOKEN para o buffer
    memcpy(((*buffer)+offset), &size_of_token_htonl, TOKEN_STRING_SIZE);
    //2.3 Atualiza offset
    offset+=TOKEN_STRING_SIZE;
    
    //3. Copia o TOKEN para o buffer
    memcpy((buffer[0]+offset), token, size_of_token);
    //3.1 Atualiza offset
    offset+=size_of_token;
    

    //4.1 Verifica se o que recebeu é maior do que a memória disponivel
    if ( offset > serialized_token_size) {
        free (*buffer);
        return TASK_FAILED;
    }
    

    //5. Devolve o tamanho do buffer
    return serialized_token_size;
}

/*
 * Deserializes a given token (Criado para Projeto 5)
 */
char* token_deserialize(char* buffer, int size) {
    
    if ( buffer == NULL ) {
        puts("token_deserialize > buffer == NULL ");
        return NULL;
    }
    
    //para gerir preenchimento do buffer
    int offset = 0;
    
    //1. obtem o tamanho do TOKEN a ler
    //   [TOKEN_DIM][TOKEN]
    int size_of_token_from_buffer = 0;
    //1.1 copia para size_of_token o tamanho do token a deserializar
    memcpy(&size_of_token_from_buffer, buffer+offset, TOKEN_STRING_SIZE);
    //1.2 converte de formato de rede o tamanho do token
    int size_of_token = ntohl(size_of_token_from_buffer);
    //1.3 Atualiza o offset
    offset+=TOKEN_STRING_SIZE;
    
    //2. Aloca memória para receber o TOKEN
    char * token_rcvd = (char*) malloc(size_of_token);
    //2.1 copia para token_rcvd o que recebe do buffer
    memcpy(token_rcvd, (buffer+offset), size_of_token);

    
    offset+= size_of_token;
    //2.2 Verifica se o que recebeu é maior do que a memória disponivel
    if ( offset > size) {
        puts("!token_deserialize >  if ( offset > size) ");
        free (token_rcvd);
        return NULL;
    }
    
    //devolve o token
    return token_rcvd;
}

long long swap_bytes_64(long long number) {
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
