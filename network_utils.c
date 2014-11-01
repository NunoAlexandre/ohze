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
#include <netdb.h> //hostent
#include "general_utils.h"
#include "network_utils.h"

/*
 * address_and_port is a string containing a host address and a port
 * with the format host_address:port.
 * returns port.
 */
char * get_port (const char * address_and_port) {
    char * address_and_port_pointer = strdup(address_and_port);
    strtok(address_and_port_pointer, ":");
    char * port = strdup(strtok(NULL,":"));
    free(address_and_port_pointer);
    return port;
}

char * get_address (const char * address_and_port) {
    //guarda valor de IP ou endereço de servidor
    char * address_and_port_pointer = strdup(address_and_port);
    // fica com o endereço
    char * address = strdup(strtok(address_and_port_pointer, ":"));
    free(address_and_port_pointer);
    return address;
}

int reads_server_portnumber ( const char * stringWithPortNumber ) {
    return atoi(stringWithPortNumber);
}

int portnumber_is_invalid (int portNumber ) {
    return portNumber <= 0 || ((portNumber >=1 && portNumber<=1023) || (portNumber >=49152 && portNumber<=65535));
}


/*
 Get ip from domain name
 */
int hostname_to_ip(char * hostname , char* ip){
    
    struct hostent *host_entry;
    struct in_addr **addr_list;
    int i;
    
    if ( (host_entry = gethostbyname( hostname ) ) == NULL){
        // get the host info
        puts("HOSTNAME TO IP ERROR: ");
        return TASK_FAILED;
    }
    
    addr_list = (struct in_addr **) host_entry->h_addr_list;
    
    for(i = 0; addr_list[i] != NULL; i++){
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return ip[i];
    }
    
    return TASK_FAILED;
}

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
            perror("network_server > read_all > failed");
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
    
    printf("Sending message: "); message_print(messageToSend); printf("\n");
    
    //creates the message buffer to send to the cliente
    char ** messageToSend_buffer = (char**) calloc(1, sizeof(char*));
    int message_size = message_to_buffer(messageToSend, messageToSend_buffer);

    if ( message_size <= 0 || message_size > MAX_MSG )
        return TASK_FAILED;
    
    int message_size_n = htonl(message_size);
    //sends the size of the message
    if ( write_all(connection_socket_fd, &message_size_n, BUFFER_INTEGER_SIZE) != BUFFER_INTEGER_SIZE ) {
        return TASK_FAILED;
    }
    //and sends the message
    if ( write_all(connection_socket_fd, *messageToSend_buffer, message_size ) != message_size ) {
        return TASK_FAILED;
    }
        
    return TASK_SUCCEEDED;
}


/*
 * Receives an integer with conection_socket_fd.
 * In error case returns NULL, received_message otherwise.
 */
struct message_t* receive_message (int connection_socket_fd) {
    
    int size_of_msg_received;
    struct message_t *message_to_receive = NULL;
    char **message_buffer = (char**) calloc(1, sizeof(char*));
    message_buffer[0] = (char*) calloc(1, MAX_MSG);
    
    /* 2.1 tamanho da mensagem que irá ser recebida*/
    if ( (read_all(connection_socket_fd,&size_of_msg_received, BUFFER_INTEGER_SIZE)) != BUFFER_INTEGER_SIZE ) {
        perror("RECEIVED MESSAGE -> FAILED TO READ MESSAGE SIZE.");
        free(message_buffer[0]);
        free(message_buffer);
        close(connection_socket_fd);
        return message_to_receive;
    }
     /* 2.2 Converte tamanho da mensagem para formato cliente */
    int size_of_msg_received_NTOHL = ntohl(size_of_msg_received);
    if ( size_of_msg_received_NTOHL  <= 0 ) {
        close(connection_socket_fd);
        free(message_buffer[0]);
        free(message_buffer);
        return message_to_receive;
    }
    
    /* 2.3 Lê mensagem enviada */
    if( (read_all(connection_socket_fd,message_buffer[0], size_of_msg_received_NTOHL) != size_of_msg_received_NTOHL)  ) {
        perror("RECEIVED MESSAGE -> FAILED TO READ MESSAGE.");
        free(message_buffer[0]);
        free(message_buffer);
        close(connection_socket_fd);
        return message_to_receive;
    }
    
    /* 2.4 Marca a terminação da Mensagem recebida */
    message_buffer[size_of_msg_received_NTOHL+1] = '\0';
    
    /* 2.5 Converte buffer para Mensagem */
    message_to_receive = buffer_to_message(message_buffer[0], size_of_msg_received);
    free(message_buffer[0]);
    free(message_buffer);

    /* 2.6 Verifica se a mensagem foi bem criada */
    if ( message_to_receive == NULL ) {
        perror("RECEIVED MESSAGE -> FAILED TO DESERIALIZE MESSAGE.");
        close(connection_socket_fd);
        return message_to_receive;
    }
    
    printf("Received message: "); message_print(message_to_receive); printf("\n");

    return message_to_receive;
}

