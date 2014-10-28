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
#include <signal.h>

int server_run ( int portnumber ) {

    /** 0. SIGPIPE Handling */
    struct signaction s;
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
    socklen_t * client_socket_size;

    //4. Accepts clients connects and handles its requests
    while ( (connection_socket_fd = accept(socket_fd, (struct sockaddr *) &client, client_socket_size)) != TASK_FAILED)
    {
             //5. receive_send
            // Handles requests from the cliente
            // and gives a proper response.
        printf("is waiting for client requests\n");
    }

    
    
    //closes server socket
    close(socket_fd);

    return TASK_SUCCEEDED;
}





int main() {
    
    server_run(1300);

}