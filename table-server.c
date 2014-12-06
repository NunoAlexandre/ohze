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
        #include "client_stub-private.h"
        #include "client_stub.h"
        #include "network_cliente.h"


        #define N_MAX_CLIENTS 4
        #define POLL_TIME_OUT 10
        #define N_TABLE_SLOTS 7

    int get_open_slot(struct pollfd * connections, int connected_fds) {
    int open_slot = connected_fds;
    int i;
    for ( i = 1; i < connected_fds; i++) {
        if ( connections[i].fd == -1 )
            open_slot = i;
    }
    return open_slot;
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

    int get_highest_open_connection ( struct pollfd * connections, int currentHighest ) {
    int i = currentHighest;
    int highest = i-1;
    int stillSearching = YES;
    while ( (i-- > 1) && stillSearching ) {
        if ( connections[i].fd != -1 ) {
            highest = i;
            stillSearching = NO;
        }
    } 
    return highest;
    }

    int server_run ( char * my_address_and_port ) {


    //gets the port number
    int portnumber = atoi(get_port(my_address_and_port));
    //case its invalid
    if ( portnumber_is_invalid(portnumber) ) {
        invalid_input_message();
        return TASK_FAILED;
    }
    char * my_address = get_address(my_address_and_port);

    printf("\n> SD15_SERVER is waiting connections at port %d\n", portnumber);


    char** system_rtables = NULL;
    int numberOfServers = get_system_rtables_info(SYSTEM_CONFIGURATION_FILE,  &system_rtables);

    int n = 0;
    for ( n = 0; n < numberOfServers; n++) {
        printf("rtable %d has address_port %s \n", n, system_rtables[n]);
    }

    if ( numberOfServers == TASK_FAILED || system_rtables == NULL )
        return TASK_FAILED;

    int switchIAM = strcmp(system_rtables[0], my_address_and_port) == 0;


    if ( switchIAM) {
        printf("\n\n ****** I AM THE SWITCH AND YOU KNOW IT! %s:%d ******\n\n", my_address,portnumber);
        
    }
    else {
        puts("IM A SUPER SERVER, NOT A SWITCH");
    }

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
    //saves the highet connection index
    int highestIndexConnection = 0;
    // to save the result from poll function
    int polled_fds = 0; 

    /**** TEMPORARY *****/
    int TEMP_UNIQUE = 0;
    //the remote table to consult
    struct rtable_t *rtable_to_consult = NULL;


            // Gets clients connection requests and handles its requests
    printf("\n--------- waiting for clients requests ---------\n");
    while ((polled_fds = poll(connections, connected_fds, 50)) >= 0) {
                //if there was any polled sockets fd with events
        if ( polled_fds > 0 ) { 
                     if ( (connections[0].revents & POLLIN) && (connected_fds < N_MAX_CLIENTS) ) {  // Pedido na listening socket ?
                        int open_slot = get_open_slot(connections, connected_fds); 
                        if ((connections[open_slot].fd = accept(connections[0].fd, (struct sockaddr *) &client, &client_socket_size)) > 0){ // Ligação feita ?
                            connections[open_slot].events = POLLIN; // Vamos esperar dados nesta socket
                            connected_fds++;
                            //updates the highest index connection if needed.
                            if ( open_slot > highestIndexConnection )
                                highestIndexConnection = open_slot;
                        }
                    }

                    //for each connected cliente it will receive a request and give a response
                    int i = 0;
                    for (i = 1; i <= highestIndexConnection; i++) {// Todas as ligações

                        if ( switchIAM ) {
                            printf("\n\n ****** I AM THE SWITCH AND YOU KNOW IT! %s:%d ******\n\n", my_address,portnumber);
                            
                            int taskSuccess = YES;
                            //to keep the active
                            //para guardar o comando do utilizador
                            
                            
                            /* 2. BINDS WITH RTABLE_TO_CONSULT */
                            if ( TEMP_UNIQUE == 0 ) {
                                rtable_to_consult = rtable_bind( system_rtables[1]);
                                printf("Switch > just binded to %s \n", rtable_to_consult->server_address_and_port);
                                TEMP_UNIQUE = 1;

                            }
                            
                            //verifica se o rtable_bind funcionou
                            if (rtable_to_consult == NULL) {
                                printf("rtable_to_consult == NULL");
                                taskSuccess = TASK_FAILED;
                            }
                            
                            //ligação foi bem sucessida
                            puts("switch will server_receive_request");
                            struct message_t * msg_request  = server_receive_request(connections[1].fd);
                            
                            /* 
                            Em qualquer dos casos em que é recebido de um table_client um comando indevido, 
                            deverá ser enviada ao cliente uma mensagem, com opcode OC_REPORT e conteúdo 
                            CT_INVCMD (definido com o valor 600), onde será enviada uma string de acordo com a ocorrência.         
                            */
                            struct message_t * msg_response = NULL;
                            
                                if ( message_is_reader(msg_request) ) {
                                    puts("\t--- switch doesnt read!");
                                    char * error_message = "switch doesnt read!";
                                    msg_response = message_create_with(OC_REPORT, CT_INVCMD, error_message);
                                }
                                else {
                                     if ( msg_request->opcode == OC_OUT) {
                                    puts("its an out, it will turn into entry with timestamp");
                                    struct entry_t * entry_to_send = entry_create2(msg_request->content.tuple, 3);
                                    struct message_t * redone_msg = message_create_with(msg_request->opcode, CT_ENTRY, entry_to_send);
                                    msg_request = redone_msg;
                                    }

                                     msg_response = network_send_receive(&(rtable_to_consult->server_to_connect), msg_request);
                                }
                                    
                                    server_send_response(connections[1].fd, 1, &msg_response);
                                    
                                    //se pedido falhou e não é para repetir pedido tenta terminar de forma controlada
                                    if (taskSuccess == TASK_FAILED){
                                        puts("\t--- failed to consult table\n");
                                    }
                            
                        }
                        else {
                        //flag to check if socket is on or was closed on client side.
                        int socket_is_on = YES;
                        // checks if this socket was closed on the client side. 
                        // If it's closed, it sets it to -1 and 
                        //decrements the number of connected_fds
                        if ( connections[i].fd != -1 && socket_is_closed(connections[i].fd) ) {
                            socket_is_on = NO;
                            //if it was not reseted yet...
                            close(connections[i].fd);
                            connections[i].fd = -1;
                            connections[i].events = 0;
                            connections[i].revents = 0;
                            connected_fds--;
                            
                            //if this was the highestIndexConnection now its the previous one
                            if ( i == highestIndexConnection )
                                highestIndexConnection = get_highest_open_connection(connections, highestIndexConnection);
                        }

                       if ( (connections[i].revents & POLLIN) && socket_is_on ) { // Dados para ler ?

                        connection_socket_fd = connections[i].fd;

                //flag to track errors during the request-response process
                        int failed_tasks = 0;

                /** Gets the client request **/
                        struct message_t * client_request = server_receive_request(connection_socket_fd);

                        puts("\t server: received "); message_print(client_request); puts("");
                //error case
                        failed_tasks += client_request == NULL;

                /** where all the response message will be stored **/
                        struct message_t ** response_message = NULL;

                //the table_skel will process the client request and resolve response_message
                        int response_messages_num = invoke(client_request, &response_message);
                // error case
                        failed_tasks+= response_messages_num <= 0 || response_message == NULL;


                        sleep(1);

                //sends the response to the client
                        int message_was_sent = server_send_response(connection_socket_fd, response_messages_num, response_message);
                //error case
                        failed_tasks+= message_was_sent == TASK_FAILED;


                /** IF some error happened, it will notify the client **/
                        if ( failed_tasks > 0 ) {

                            server_sends_error_msg(connection_socket_fd);

                        }

                /** frees memory **/
                        free_message2(client_request, NO);
                        free_message_set(response_message, response_messages_num);

                    }
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



    int main ( int argc, char *argv[] ) {
        
        char * address_and_port = strdup(argv[1]);
        
        if ( server_run(address_and_port) == TASK_FAILED ) {
            puts("server_run(address_and_port) failed or crashed (TASK_FAILED)");
        };


        return TASK_SUCCEEDED;
    }