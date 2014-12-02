//
//  general_utils.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 30/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include "inet.h"
#include "general_utils.h"
#include <string.h>

int is_number (char * stringWithNumber ) {
    char *ptr;
    strtol(stringWithNumber, &ptr, 10);
    
    return strncmp(ptr, "", 1) == 0;
}

/*
* Gets all the servers from the file of filePath and saves them into all_servers
* Saves the switch server at the first position of all_servers and the other servers
* on the following ones.
* IF there is no switch defined or the number of servers found 
* is not equal to what is announced, returns TASK_FAILED, otherwise TASK_SUCCEDDED;
*/
int get_all_servers(char * filePath, int *number_of_servers, struct server_t *** all_servers ) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(filePath, "r");
	if (fp == NULL)
		return TASK_FAILED;


	char * pointer = NULL;

	//reads the first line that informs the number of servers
	if ( (read = getline(&line, &len, fp)) == TASK_FAILED ) {
		return TASK_FAILED;
	}
	//gets the number of servers itself
	strtok_r(line,"=", &pointer);
	*number_of_servers = atoi(pointer);

	//allocs memory for all
	*all_servers = malloc ( sizeof(struct server_t *) * *number_of_servers);

	int lookForSwitchServer = YES;
	int iServer = 1;
	//then it will read all the lines 
	while ((read = getline(&line, &len, fp)) != -1) {
		if ( lookForSwitchServer ) {
			lookForSwitchServer = !get_switch_server_from(line, *all_servers[0]);
		}
		else {
			get_server_from(line, *all_servers[iServer]);
			iServer++;
		}
	}

	fclose(fp);
	if (line)
		free(line);

	return TASK_SUCCEEDED;
}

int get_server_from(char * lineWithServerInfo,  struct server_t * server) {
	char * line = strdup(lineWithSwitchInfo);

	server->ip_address = get_port(line);
	server->port = get_address(line);
	server->socketfd = -1;

	free(line);

	return YES;
}

int get_switch_server_from(char * lineWithSwitchInfo,  struct server_t * server) {
	int switch_founded = NO;
	char * line = strdup(lineWithSwitchInfo);
	char * switch_identifier = NULL;
	strtok_r(line, " ", &switch_identifier);
	char * breakLine = NULL;
	strtok_r(switch_identifier, "\n", &breakLine);
	if ( strcmp(switch_identifier, SWITCH_SERVER_IDENTIFIER) == 0 ) {
		server->ip_address = get_port(line);
		server->port = get_address(line);
		server->socketfd = -1;
		switch_founded = YES;
	}
	
	free(line);
	free(switch_identifier);
	free(breakLine);

	return switch_founded;
}

