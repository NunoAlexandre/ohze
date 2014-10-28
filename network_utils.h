//
//  network_utils.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 28/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#ifndef SD15_Product_network_utils_h
#define SD15_Product_network_utils_h

int write_all(int socket_fd, const void *buffer, int bytesToWrite);

int send_message (int connection_socket_fd, struct message_t * messageToSend);

#endif
