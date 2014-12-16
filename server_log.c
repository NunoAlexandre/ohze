//
//  log.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 13/12/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "general_utils.h"
#include "message.h"
#include "server_log.h"
#include "network_utils.h"
#include "table_skel.h"
#include "table.h"



FILE *_log_fd;
char * _log_file;


/*
 * checks if the server_log_file exists and is accessable to read&write
 */
int server_log_setup_file() {
    _log_fd = fopen(_log_file, "ab+");
    return _log_fd != NULL;
}

/*
 * initializes the log
 */
void server_log_init( char * filepath ) {
    server_log_file_create(filepath);
    if (server_log_setup_file() == FAILED );
}




void server_log_file_create(char * filepath) {
    int fname_size = (int) strlen(filepath) + 8 + 1;
    _log_file = malloc ( fname_size);
    malloc ( fname_size );
    _log_file[0] = '\0';   // ensures the memory is an empty string
    strcat(_log_file,filepath);
    strcat(_log_file,"_LOG.txt");
}

/*
 * Logs buffer message into the server_log_file
 */
int server_log_write(char * buffer, int n_bytes ) {
    
    FILE* fp = fopen(_log_file, "a");
    if ( fp == NULL )
        return FAILED;
    
    fprintf(fp, "%s\n",buffer);
    
    if ( buffer != NULL )
        free(buffer);
    
    fclose(fp);
    
    return SUCCEEDED;
}

int server_log_message( struct message_t * message ) {
    char * buffer = message_to_string(message);
    return server_log_write(buffer, (int) strlen(buffer));
}

/*
 * Returns a message_t * built from the command string.
 * Assumes the command is valid.
 */
struct message_t * server_log_to_message (const char * command, int callFromServer ) {
    //get opcode
    int opcode = find_opcode_as_int(command);
    
    //get ctype
    int ctype = find_ctype(command);
    
    void * message_content = NULL;
    
    if ( ctype == CT_TUPLE ) {
        message_content = create_tuple_from_input (command);
    }
    else if ( ctype == CT_ENTRY ) {
        message_content = entry_create_from_string(command);
    }
    
    else if ( ctype == CT_RESULT ) {
        int resultValue = 0;
        message_content = &resultValue;
    }
    
    //create and return message
    struct message_t * message = message_create_with(opcode, ctype, message_content);
    
    return message;
}

int server_log_invoke_over_table(struct table_t * table) {
    
    if ( table==NULL)
        return FAILED;
    
    char * line = NULL;
	size_t len = 0;
	ssize_t read;
    
    FILE* fp = fopen(_log_file, "r");
    if (fp == NULL)
		return 0;
    
    while( (read = getline(&line, &len, fp) ) != -1 ) {
        struct message_t * operation = server_log_to_message(line, YES);
        struct message_t ** msg_out = NULL;
        invoke(operation, &msg_out);
        free_message2(operation, NO);
    }
    
    return SUCCEEDED;
}

int server_log_send_to ( int addressee_fd, int from_operation_n ) {
    
    char * line = NULL;
	size_t len = 0;
	ssize_t read;
    
    FILE* fp = fopen(_log_file, "r");
    if (fp == NULL)
		return 0;
    
    int n_operation = 0;
    
    while ( (read = getline(&line, &len, fp) ) != -1 ) {
        n_operation++;
        if ( n_operation > from_operation_n ) {
            struct message_t * operation = server_log_to_message(line, YES);
            if ( send_message(addressee_fd, operation) == SUCCEEDED )
                free_message(operation);
        }
    }
    
    return SUCCEEDED;
}
//table_skel_update_neighboor

void server_log_print() {
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
    
    FILE* fp = fopen(_log_file, "r");
    if (fp == NULL)
		return;

    
    while( (read = getline(&line, &len, fp) ) != -1 )
          printf("%s\n", line);
    
    
    fclose(fp);
}


