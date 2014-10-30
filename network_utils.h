//
//  network_utils.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 28/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#ifndef SD15_Product_network_utils_h
#define SD15_Product_network_utils_h

/*
 * Ensures that all nbytesToWrite of the buffer are written to the socket_fd.
 * The only case it doesn't happen is if some EINTR EPIPE happens.
 * Returns the number of bytes written so if its different 
 * than nbytesToWrite something went wrong.
 */
int write_all(int socket_fd, const void *buffer, int nbytesToWrite);

/*
 * Ensures that all nbytesToRead are readed from the socket
 * and moved into the buffer.
 * Returns the number of bytes copied so if its different
 * than nbytesToRead something went wrong.
 */
int read_all( int socket_fd, void *buffer, int nBytesToRead );

/*
 * Sends messageToSend to the connection_socket
 */
int send_message (int connection_socket_fd, struct message_t * messageToSend);


/*
 * Receives an integer with conection_socket_fd.
 * In error case returns NULL, received_message otherwise.
 */
struct message_t* receive_message (int connection_socket_fd);

#endif
