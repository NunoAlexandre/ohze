//
//  network_server.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 26/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#ifndef SD15_Product_network_server_h
#define SD15_Product_network_server_h

#include "message-private.h"


/*
* Sends number_of_messages from response_messages to the socketfd.
*/
int server_send_response(int socketfd, int number_of_messages, struct message_t ** response_messages);
/*
* Receive_request simply receive_message from socketfd.
* Useful to improve readability.
*/
struct message_t * server_receive_request(int socketfd );
/*
* Given the socket_fd it will send an error message to it.
*/
int server_sends_error_msg( int connection_socket_fd);


#endif
