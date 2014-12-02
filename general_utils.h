//
//  general_utils.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 30/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//
#ifndef SD15_Product_general_utils_h
#define SD15_Product_general_utils_h

#include "network_client-private.h"


#include <unistd.h>

#define TASK_FAILED -1
#define TASK_SUCCEEDED 0
#define YES 1
#define NO  0

#define SWITCH_SERVER_IDENTIFIER "S"


#define CLIENT_RECEIVE_TUPLES 1
#define CLIENT_PROCEED 0

int is_number (char * stringWithNumber );

int get_all_servers(char * filePath, int *number_of_servers, struct server_t *** all_servers );
int get_switch_server_from(char * lineWithSwitchInfo,  struct server_t * server);
int get_server_from(char * lineWithServerInfo,  struct server_t * server);
#endif
