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

int get_all_servers(char * filePath, int *number_of_servers, void ** all_servers) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(filePath, "r");
	if (fp == NULL)
		return TASK_FAILED;


	char * pointer = NULL;

	if ( (read = getline(&line, &len, fp)) == TASK_FAILED ) {
		return TASK_FAILED;
	}

	strtok_r(line,"=", &pointer);
	*number_of_servers = atoi(pointer);
	printf("\n #number_of_servers is %d\n\n", *number_of_servers);

	while ((read = getline(&line, &len, fp)) != -1) {
		printf("Server info at this line is:  %s\n", line);
	}

	fclose(fp);
	if (line)
		free(line);

	return TASK_SUCCEEDED;
}



