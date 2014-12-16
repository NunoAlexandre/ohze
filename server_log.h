//
//  log.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 13/12/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#ifndef SD15_Product_log_h
#define SD15_Product_log_h

#include "table.h"

/* 
 * initializes the log
 */
void server_log_init( char * filepath );

/*
 * checks if the server_log_file exists and is accessable to read&write
 */
int server_log_setup_file();

const char * server_log_write_mode();

void server_log_file_create(char * filepath);


const char * server_log_write_mode();

/*
 * Logs buffer message into the server_log_file
 */
int server_log_write(char * buffer, int n_bytes );

int server_log_message( struct message_t * message );

int server_log_invoke_over_table(struct table_t * table);

int server_log_send_to (int addressee_fd, int from_operation_n);

void server_log_print();

#endif
