//
//  network_utils.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 28/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#ifndef SD15_Product_network_utils_h
#define SD15_Product_network_utils_h

#include "client_stub-private.h"


//time to retry to reconnect
#define RETRY_TIME 5
#define SWITCH_SERVER_IDENTIFIER "S"
#define SYSTEM_CONFIGURATION_FILE "./SD15-Project/sd15_system_config"


char* ip_address_copy_from_server;


int split_address_port (const char *address_and_port, char * address, char * port );

int reads_server_portnumber ( const char * stringWithPortNumber );

int portnumber_is_invalid (int portNumber );

int hostname_to_ip(char * hostname , char* ip);

char * get_port (const char * address_and_port);

char * get_address (const char * address_and_port);


/*
* Checks, without modifying its value, if the socket_fd is open.
*/
int socket_is_open(int socket_fd );

/*
* Checks if the pointed socketfd is open or not (YES or NO as return).
* If socket got closed, then YES.
*/
int socket_is_closed(int socket_fd );

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


int get_system_server(char * lineWithServerInfo,  char  ** system_server);
int get_system_switch(char * lineWithSwitchInfo,  char ** system_switch);
/*
 * Gets all the rtables from the system_configuration_file and saves them into system_rtables.
 * Saves the switch server at the first position (0) and the other servers on the following ones.
 * IF there is no switch defined or the number of servers found is not equal to what is announced,
 * returns TASK_FAILED, otherwise returns the number of rtables of the system;
 */
int get_system_rtables_info(char * filePath, char *** system_rtables );


#endif
