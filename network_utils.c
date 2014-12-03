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

int socket_is_open(int socket_fd ) {
    char * buffer = malloc (1); 
    if ( recv(socket_fd, buffer, 1, MSG_PEEK | MSG_DONTWAIT) == 0 ) {
        socket_fd = -1;
    }
    free(buffer);
    //if socket is not -1 then its opened 
    return socket_fd != -1;
}

int socket_is_closed(int socket_fd ) {
    return ! socket_is_open(socket_fd);
} 

/*
 * Ensures that all nbytesToWrite of the buffer are written to the socket_fd.
 * The only case it doesn't happen is if some EINTR EPIPE happens.
 * Returns the number of bytes written so if its different
 * than nbytesToWrite something went wrong.
 */
int write_all(int socket_fd, const void *buffer, int bytesToWrite) {
    //checks if socket is closed...
    if ( socket_is_closed(socket_fd) )
        return TASK_FAILED;

    int bufsize = bytesToWrite;

    while(bytesToWrite>0 ) {
        //checks it to avoid issues
        if ( socket_is_closed(socket_fd) )
            return TASK_FAILED;
        //if its open then it can write into it.
        int writtenBytes = (int) write(socket_fd, buffer, bytesToWrite);
        if(writtenBytes<0) {
            if(errno==EINTR) continue;
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
        //if nReadedBytes is -1 (error) or 0 (socket closed)
        if ( nReadedBytes <= 0 ) {
            if(errno==EINTR) continue;
            return TASK_SUCCEEDED;
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
    //puts("\t --- sending message ---");
    if ( messageToSend == NULL )
        return TASK_FAILED;
    
    printf("Sending message: "); message_print(messageToSend); printf(" <> %d bytes\n", message_size_bytes(messageToSend));
    
    //the buffer
    char * messageToSend_buffer = NULL;
    //the message size
    int message_size = -1;
    //fills buffer with the serialized messageToSend
    if ( (message_size = message_to_buffer(messageToSend, &messageToSend_buffer)) == -1 ) {
        printf("send message > error on message_to_buffer\n");
        return TASK_FAILED;
    }
    
    //converts it to network format
    int message_size_n = htonl(message_size);
    //sends the size of the message
    int writtenBytes = 0;
    if ( (writtenBytes = write_all(connection_socket_fd, &message_size_n, BUFFER_INTEGER_SIZE)) != BUFFER_INTEGER_SIZE ) {
        if ( writtenBytes == TASK_FAILED )
            printf("\t--- failed: socket is closed\n");

        return TASK_FAILED;
    }
    //and sends the message
    if ( write_all(connection_socket_fd, messageToSend_buffer, message_size ) != message_size ) {
        return TASK_FAILED;
    }
    //frees the local buffer
    free(messageToSend_buffer);

    return TASK_SUCCEEDED;
}


/*
 * Expects to receives an integer of 4 bytes followd by a serialized message_t from connection_socket_fd.
 * In error case returns NULL, otherwise returns the deserialized message.
 */
struct message_t* receive_message (int connection_socket_fd) {
    //puts("\t --- receiving message ---");

    int size_of_msg_received = 0;

    // 1. Lê tamanho da mensagem que a ser recebida
    if ( (read_all(connection_socket_fd,&size_of_msg_received, BUFFER_INTEGER_SIZE)) != BUFFER_INTEGER_SIZE ) {
        return NULL;
    }
     // 1.1 Converte tamanho da mensagem para formato cliente
    int size_of_msg_received_NTOHL = ntohl(size_of_msg_received);
    //safety check
    if ( size_of_msg_received_NTOHL  <= 0 || size_of_msg_received_NTOHL > MAX_MSG ) {
        puts("receive_message > message size <= 0 or > MAX_MSG");
        return NULL;
    }
    
    // 2. Recebe a mensagem
    // allocs a buffer with the size that was informed from the other host
    char * message_buffer = (char*) malloc(size_of_msg_received_NTOHL);
    
    // lê a mensagem enviada
    if( (read_all(connection_socket_fd,message_buffer, size_of_msg_received_NTOHL) != size_of_msg_received_NTOHL)  ) {
        puts("receive_message -> failed to read message\n");
        free(message_buffer);
        return NULL;
    }
    
    // 2.4 Converte buffer para Mensagem
    //printf("Message to receive has %d bytes\n", size_of_msg_received_NTOHL);
    //puts("receive_message > before buf_to_msg");
   // printf("size_of_msg_received_NTOHL is %d\n", size_of_msg_received_NTOHL);
    struct message_t * message_received = buffer_to_message(message_buffer, size_of_msg_received_NTOHL );

    // 2.5 Verifica se a mensagem foi bem criada */
    if ( message_received == NULL ) {
        puts("receive_message -> failed to buffer_to_message (returned null)\n");
        free(message_buffer);
        return NULL;
    }
    
    printf("Received message: "); message_print(message_received); printf(" <> %d bytes\n", message_size_bytes(message_received));
    
    //frees the local buffer
    free(message_buffer);
 
    return message_received;
}


int get_system_server(char * lineWithServerInfo,  struct rtable_t ** system_server) {

	char * line = strdup(lineWithServerInfo);

    *system_server = malloc( sizeof(**system_server));
	(*system_server)->server_address_and_port = line;
    (*system_server)->server_to_connect.ip_address =get_address(line);
    char * portnumber = get_port(line);
	 (*system_server)->server_to_connect.port =  atoi(portnumber);
    free(portnumber);
	 (*system_server)->server_to_connect.socketfd = -1;
    
	return YES;
}

int get_system_switch(char * lineWithSwitchInfo,  struct rtable_t ** system_switch) {
    
    int switch_founded = NO;
	char * line = strdup(lineWithSwitchInfo);
	char * switch_identifier = NULL;
	strtok_r(line, " ", &switch_identifier);
    if ( switch_identifier == NULL) {
        free(line);
        return NO;
    }
	char * breakLine = NULL;
	strtok_r(switch_identifier, "\n", &breakLine);
    
	if ( strcmp(switch_identifier, SWITCH_SERVER_IDENTIFIER) == 0 ) {
        get_system_server( line,  system_switch);
        switch_founded = YES;
	}

    if ( line != NULL)
        free(line);
    
	return switch_founded;
}


/*
 * Gets all the rtables from the system_configuration_file and saves them into system_rtables.
 * Saves the switch server at the first position (0) and the other servers on the following ones.
 * IF there is no switch defined or the number of servers found is not equal to what is announced,
 * returns TASK_FAILED, otherwise returns the number of rtables of the system;
 */
int get_system_rtables(char * system_configuration_file, struct rtable_t *** system_rtables ) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
    
	fp = fopen(system_configuration_file, "r");
	if (fp == NULL)
		return TASK_FAILED;
    
    
	char * pointer = NULL;
    
	//reads the first line that informs the number of servers
	if ( (read = getline(&line, &len, fp)) == TASK_FAILED ) {
		return TASK_FAILED;
	}
	//gets the number of servers itself
	strtok_r(line,"=", &pointer);

	int number_of_servers = atoi(pointer);

    
	//allocs memory for all
	*system_rtables = malloc ( sizeof(**system_rtables) * number_of_servers );

	int switchNotFoundYet = YES;
	int iServer = 1;
    int serversFound = 0;
    int switchWasFoundNow = NO;
	//then it will read all the lines
	while ((read = getline(&line, &len, fp)) != -1) {
        
		if ( switchNotFoundYet ) {
			switchNotFoundYet = !get_system_switch(line, &(*system_rtables)[0]);

            if ( !switchNotFoundYet ) {
                serversFound++;
                switchWasFoundNow = YES;
            }
		}
		if ( !switchWasFoundNow ) {
             if ( get_system_server(line, &(*system_rtables)[iServer]) ) {
                iServer++;
                serversFound++;
            }
		}
        switchWasFoundNow = NO;
	}
    
	fclose(fp);
	if (line)
		free(line);
    
	return (!switchNotFoundYet) && (serversFound == number_of_servers) ? number_of_servers : TASK_FAILED;
}


