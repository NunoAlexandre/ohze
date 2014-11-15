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
#include "table_skel.h"
#include <sys/poll.h>
#include <limits.h>
#include <sys/uio.h>


#define N_MAX_CLIENTS 10
#define POLL_TIME_OUT 10
#define N_TABLE_SLOTS 7

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
    
    //sets the socket reusable
    int setSocketReusable = YES;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (int *)&setSocketReusable,sizeof(setSocketReusable)) < 0 ) {
        perror("SO_REUSEADDR setsockopt error");
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
        perror("server > server_run > error on listen() \n");
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

    // initializes the table_skel
    if ( table_skel_init(N_TABLE_SLOTS) == TASK_FAILED )
        return -1;


    /** creates a pollfd **/ 
    struct pollfd connections[N_MAX_CLIENTS];
    //the socket_fd is at first
    connections[0].fd = socket_fd;
    connections[0].events = POLLIN;

    int i = 1;
    for( i=1; i < N_MAX_CLIENTS; i++){
        connections[i].fd = -1;
        connections[i].events = 0;
        connections[i].revents = 0;
    }

    //for now only the listening socket
    int connected_fds = 1;
    // to save the result from poll function
    int polled_fds = 0; 

    // Gets clients connection requests and handles its requests
    printf("\n--------- waiting for clients requests ---------\n");
    while ((polled_fds = poll(connections, connected_fds, 50)) >= 0) {
        //if there was any polled sockets fd with events
        if (polled_fds > 0){ 
             if ((connections[0].revents & POLLIN) && (connected_fds < N_MAX_CLIENTS)) {  // Pedido na listening socket ?
                if ((connections[connected_fds].fd = accept(connections[0].fd, (struct sockaddr *) &client, &client_socket_size)) > 0){ // Ligação feita ?
                    connections[connected_fds].events = POLLIN; // Vamos esperar dados nesta socket
                    connected_fds++;
                }
            }

            //for each connected cliente it will receive a request and give a response
            int i = 0;
            for (i = 1; i < connected_fds; i++) {// Todas as ligações

               if (connections[i].revents & POLLIN && connections[i].fd != -1) { // Dados para ler ?

                connection_socket_fd = connections[i].fd;
                printf("> serving client with socket_fd %d\n", connection_socket_fd);

        //flag to track errors during the request-response process
                int failed_tasks = 0;

        /** Gets the client request **/
                struct message_t * client_request = server_receive_request(connection_socket_fd);
        //error case
                failed_tasks += client_request == NULL;

        /** where all the response message will be stored **/
                struct message_t ** response_message = NULL;

        //the table_skel will process the client request and resolve response_message
                int response_messages_num = invoke(client_request, &response_message);
        // error case
                failed_tasks+= response_messages_num <= 0 || response_message == NULL;

        //sends the response to the client
                int message_was_sent = server_send_response(connection_socket_fd, response_messages_num, response_message);
        //error case
                failed_tasks+= message_was_sent == TASK_FAILED;


        /** IF some error happened, it will notify the client **/
                if ( failed_tasks > 0 ) 
                    server_sends_error_msg(connection_socket_fd);

        /** frees memory **/
                free_message2(client_request, NO);
                free_message_set(response_message, response_messages_num);
            }
        }
        }
    }

    //closes all the sockets socket
    int j;
    for (j = 0; j < N_MAX_CLIENTS; j++){
        if (connections[j].fd >= 0){
            close(connections[j].fd);
        }
    }
    //destroys the table_skel
    table_skel_destroy();

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