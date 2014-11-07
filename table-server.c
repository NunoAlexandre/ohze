//
//  table-server.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 25/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include "inet.h"
#include "table-private.h"
#include "network_server.h"
#include "general_utils.h"
#include "network_utils.h"
#include <signal.h>

int server_run ( int portnumber ) {

    /** 0. SIGPIPE Handling */
    struct sigaction s;
    //what must do with a signal - ignore
    s.sa_handler = SIG_IGN;
    //set what to do when gets the SIGPIPE
    sigaction(SIGPIPE, &s, NULL);

    //1 . Socket
    int socket_fd;
    
    //creates a server socket
    if (  (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        //error case
        perror("server > server_run > error creating socket\n");
        return TASK_FAILED;
    }
    
    //2. Bind
    struct sockaddr_in server;
    
    //initializes server sockaddr_in
    server.sin_family = AF_INET;
    server.sin_port = htons(portnumber);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //does the bind
    if (bind(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("server > server_run > error binding socket\n");
        close(socket_fd);
        return TASK_FAILED;
    };
    
    //3. Listen
    if (listen(socket_fd, 0) < 0 ) {
        perror("server > server_run > error on listen()\n");
        close(socket_fd);
        return TASK_FAILED;
    };
    
    
    
    /*  From now on the server will wait that clients
         send requests that will be receive_and_send. */
    
    //sockaddres_in struct for a client
    struct sockaddr_in client;
    //the connection socket with a client
    int connection_socket_fd;
    //the cliente socket size
    socklen_t  client_socket_size = sizeof (client);

    //4. Accepts clients connects and handles its requests
    
    struct table_t * server_table = table_create(TABLE_DIMENSION);
    
    
    do  {
        printf("\n--------- waiting for clients requests ---------\n");
        connection_socket_fd = accept(socket_fd, (struct sockaddr *) &client, &client_socket_size);
        printf("> cliente connected.\n");
        network_receive_send ( server_table , connection_socket_fd );
       // close (connection_socket_fd);
    }
    while ( connection_socket_fd != -1 );

    //closes server socket
    close(socket_fd);

    return TASK_SUCCEEDED;
}



int input_is_valid (int argc, char *argv[]) {
    return  argc > 1 && is_number (argv[1]);
}



void invalid_input_message () {
    puts("####### SD15-SERVER ##############");
    puts("Sorry, your input was not valid.");
    puts("You must provide a valid number to be the server port.");
    puts("NOTE: Port invalid if (portNumber >=1 && portNumber<=1023) OR (portNumber >=49152 && portNumber<=65535)");
    puts("####### SD15-SERVER ##############");
}

int main ( int argc, char *argv[] ) {
    
    //gets the port number
    int portNumber = input_is_valid(argc, argv) ? reads_server_portnumber(argv[1]) : -1;
    //case its invalid
    if ( portnumber_is_invalid(portNumber) ) {
        invalid_input_message();
        return TASK_FAILED;
    }
    printf("\n> SD15_SERVER is waiting connections at port %d\n", portNumber);
    
    server_run(portNumber);
    

    return TASK_SUCCEEDED;
}