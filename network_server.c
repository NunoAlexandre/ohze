//
//  network_server.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 26/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//
#include <stdio.h>
#include <errno.h>
#include "inet.h"
#include "table-private.h"
#include "message.h"
#include "message-private.h"
#include "tuple-private.h"
#include "network_utils.h"
#include "tuple.h"
#include "general_utils.h"


/*
* Receive_request simply receive_message from socketfd.
* Useful to improve readability.
*/
struct message_t * server_receive_request(int socketfd ) {
    return receive_message(socketfd);
}

int server_send_response(int socketfd, int number_of_messages, struct message_t ** response_messages) {
    if ( number_of_messages <= 0 || response_messages == NULL ) {
        return TASK_FAILED;
    }
    
    int taskSuccess = TASK_SUCCEEDED;
    int index = 0;
    //sends each message. in error case stops to send.
    while ( index >= 0 && index < number_of_messages ) {
        if ( send_message(socketfd, response_messages[index]) == TASK_FAILED ) {
            taskSuccess = TASK_FAILED;
            index = -1;
        }
        else {
            index++;
        }
    }
    return taskSuccess;
}

int server_sends_error_msg( int connection_socket_fd ) {
    struct message_t * errorMessage = message_of_error();
    int taskSuccess = send_message(connection_socket_fd, errorMessage);

    if ( errorMessage != NULL )
        free_message(errorMessage);
    
    return taskSuccess;
}

