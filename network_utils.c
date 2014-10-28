//
//  network_utils.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 28/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include "inet.h"
#include <errno.h>
#include "list-private.h"
#include "message-private.h"

/*
 * Ensures that all nbytesToWrite of the buffer are written to the socket_fd.
 * The only case it doesn't happen is if some EINTR EPIPE happens.
 * Returns the number of bytes written so if its different
 * than nbytesToWrite something went wrong.
 */
int write_all(int socket_fd, const void *buffer, int bytesToWrite) {
    
    int bufsize = bytesToWrite;
    
    while(bytesToWrite>0) {
        int writtenBytes = (int) write(socket_fd, buffer, bytesToWrite);
        if(writtenBytes<0) {
            if(errno==EINTR) continue;
            perror("network_server > write_all > failed");
            return writtenBytes;
        }
        //moves buffer pointer
        buffer += writtenBytes;
        //bytes to write
        bytesToWrite -= writtenBytes;
    }
    return bufsize;
}

/*
 * Ensures that all nbytesToRead are readed from the socket
 * and moved into the buffer.
 * Returns the number of bytes copied so if its different
 * than nbytesToRead something went wrong.
 */
int read_all( int socket_fd, void *buffer, int nBytesToRead ) {
    
    int bufsize = nBytesToRead;
    
    while ( nBytesToRead > 0 ) {
        int nReadedBytes = (int) read(socket_fd, buffer, nBytesToRead);
        if ( nReadedBytes < 0 ) {
            if(errno==EINTR) continue;
            perror("network_server > write_all > failed");
            return nReadedBytes;
        }
        //moves buffer pointer
        buffer += nReadedBytes;
        //bytes to write
        nBytesToRead -= nReadedBytes;
    }
    return bufsize;
}

/*
 * Sends a given message to the connection_socket_fd.
 * First sends an integer with the message buffer size and then the message itself.
 * In error case returns TASK_FAILED, TASK_SUCCEEDED otherwise.
 */
int send_message (int connection_socket_fd, struct message_t * messageToSend) {
    
    if ( messageToSend == NULL )
        return TASK_FAILED;
    
    //creates the message buffer to send to the cliente
    char ** messageToSend_buffer = (char**) calloc(1, sizeof(char*));
    int message_size_n = htonl(message_to_buffer(messageToSend, messageToSend_buffer));
    
    if ( message_size_n <= 0 || message_size_n > MAX_MSG )
        return TASK_FAILED;
    
    //sends the size of the message
    if ( write_all(connection_socket_fd, &message_size_n, BUFFER_INTEGER_SIZE) != BUFFER_INTEGER_SIZE ) {
        return TASK_FAILED;
    }
    //and sends the message
    int msg_buffer_size = (int) strlen(*messageToSend_buffer);
    if ( write_all(connection_socket_fd, *messageToSend_buffer, msg_buffer_size ) != msg_buffer_size ) {
        return TASK_FAILED;
    }
    
    return TASK_SUCCEEDED;
}
