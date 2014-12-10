//
//  table-server.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 25/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//system_rtables

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
#include "table_skel-private.h"
#include "server_proxy.h"
#include "message-private.h"
#include <pthread.h>
#include <time.h>


#define N_MAX_CLIENTS 25
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



struct message_t *request_to_switch_mode ( struct message_t * original ) {
    if ( message_opcode_setter(original) && original->c_type == CT_TUPLE ) {
        time_t timePassed;
        time ( &timePassed );
        struct entry_t * entry = entry_create2(tuple_dup(original->content.tuple), (timePassed%100) );
        return message_create_with(original->opcode, CT_ENTRY, entry);
    }

    return original;
}

struct message_t * respond_to_report ( struct message_t * report, void * useful_info ) {
    struct message_t * response_msg = NULL;
    if ( report ->c_type == CT_SFAILURE ) {
        response_msg = message_create_with(report->opcode+1, CT_SRUNNING, useful_info);
    }
    return response_msg;
}









 int server_run ( char * my_address_and_port, char ** system_rtables, int numberOfServers ) {

    //gets the port number
    int portnumber = atoi(get_port(my_address_and_port));
    //case its invalid
    if ( portnumber_is_invalid(portnumber) ) {
        invalid_input_message();
        return TASK_FAILED;
    }

    printf("\n> SD15_SERVER is waiting connections at port %d\n", portnumber);



    int n = 0;
    for ( n = 0; n < numberOfServers; n++) {
        printf("rtable %d has address_port %s \n", n, system_rtables[n]);
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

    // sets the socket reusable
    int setSocketReusable = YES;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (int *)&setSocketReusable,sizeof(setSocketReusable)) < 0 ) {
        perror("SO_REUSEADDR setsockopt error");
        return TASK_FAILED;
    } 

    // 2. Bind
    struct sockaddr_in server;

    //initializes server sockaddr_in
    server.sin_family = AF_INET;
    server.sin_port = htons(portnumber);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // does the bind
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
    if ( table_skel_init_with_mode( N_TABLE_SLOTS, SERVER_RESPONSE_MODE ) == TASK_FAILED )
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

            // Gets clients connection requests and handles its requests
    printf("\n--------- waiting for clients requests ---------\n");
    
    
    while ((polled_fds = poll(connections, connected_fds, 50)) >= 0) {

        //if there was any polled sockets fd with events
        if ( polled_fds > 0 ) { 

            /** enters if there is a request on the listening socket **/ 
            if ( (connections[0].revents & POLLIN) && (connected_fds < N_MAX_CLIENTS) ) {  
                /* gets an open slot*/
                int open_slot = get_open_slot(connections, connected_fds); 

                if ((connections[open_slot].fd = accept(connections[0].fd, (struct sockaddr *) &client, &client_socket_size)) > 0){ // Ligação feita ?
                    connections[open_slot].events = POLLIN; // Vamos esperar dados nesta socket
                    connected_fds++;
                    //updates the highest index connection if needed.
                    if ( open_slot > highestIndexConnection ) {
                        highestIndexConnection = open_slot;
                    }
                }
            }

            //for each connected cliente it will receive a request and give a response
            int i = 0;
            for (i = 1; i < N_MAX_CLIENTS; i++) {

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

                    int failed_tasks = 0;
                    //flag to track errors during the request-response process

                    /** Gets the client request **/
                    struct message_t * client_request = server_receive_request(connection_socket_fd);

                    //error case
                    failed_tasks += client_request == NULL;

                   
                    
                    /** where all the response message will be stored **/
                    struct message_t ** response_message = NULL;
                    int response_messages_num = 0;
                    int message_was_sent = NO;
                    
                    if ( message_report(client_request) ) {
                        struct message_t * report_response = respond_to_report(client_request, system_rtables[0]);
                        response_messages_num = 1;
                        message_was_sent = server_send_response(connection_socket_fd, response_messages_num, &report_response);
                        failed_tasks = message_was_sent == TASK_FAILED;
                    }
                    else {
                        //the table_skel will process the client request and resolve response_message
                        int response_messages_num = invoke(client_request, &response_message);
                        // error case
                        failed_tasks+= response_messages_num <= 0 || response_message == NULL;
                        

                        sleep(1);

                        //sends the response to the client
                        message_was_sent = server_send_response(connection_socket_fd, response_messages_num, response_message);
                        //error case
                        failed_tasks+= message_was_sent == TASK_FAILED;
                    }


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









int switch_run ( char * my_address_and_port, char ** system_rtables, int numberOfServers ) {
//    printf("\n> SD15_SERVER is waiting connections at port %d\n", portnumber);


    /** 0. SIGPIPE Handling */
    struct sigaction s;
    //what must do with a signal - ignore
    s.sa_handler = SIG_IGN;
    //set what to do when gets the SIGPIPE
    sigaction(SIGPIPE, &s, NULL);


    /** gets this server port number **/
    int portnumber = atoi(get_port(my_address_and_port));
    /** gets this server ip address **/
    char * my_address = get_address(my_address_and_port);

    /* if port or address invalid*/
    if ( portnumber_is_invalid(portnumber) || my_address == NULL ) {
        invalid_input_message();
        return TASK_FAILED;
    }

   

    /* if there is not at least one switch and one server */
    if ( numberOfServers <= 1 ) {
        puts("--- ERROR: starting server: no minimum services provided (switch and servers number).");
        return TASK_FAILED;
    }


    //1 . Socket
    int socket_fd;

    //creates a server socket
    if (  (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
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


    /** the number of proxies the switch will provide **/
    int NUMBER_OF_PROXIES = numberOfServers-1;
    set_number_of_proxies(NUMBER_OF_PROXIES);
    /** array with data for each thread that will be provided **/
    struct thread_data threads[NUMBER_OF_PROXIES]; 
    /** array with the ids of each thread that will be provided **/
    pthread_t thread_ids[NUMBER_OF_PROXIES];

    /** the bucket where requests will be stored and let available to all the threads **/
    struct request_t *requests_bucket[REQUESTS_BUCKET_SIZE];

    /** a place to store each in processing message **/
    struct request_t * current_request = NULL;

    /** the monitor to observe if requests_bucket has requests **/
    struct monitor_t monitor_bucket_has_requests; 
    /** the monitor to check if the bucket is not null **/
    struct monitor_t monitor_bucket_not_full;

     /** initializes the two monitors **/
    monitor_init(&monitor_bucket_has_requests);
    monitor_init(&monitor_bucket_not_full);

    /** Thy locket to ensure mutex accessing the bucket **/ 
    pthread_mutex_t bucket_access = PTHREAD_MUTEX_INITIALIZER; 

    /** Variable on the bucket status **/
    int bucket_is_full = NO;    
    int bucket_has_requests = NO;
    int requests_counter = 0;
    unsigned long long total_requests_count = 0;
    int index_to_store_request = 0;
    
    /** initializes each slot of the bucket **/
    int i;
    for (i = 0; i < REQUESTS_BUCKET_SIZE; i++) 
    requests_bucket[i] = NULL;


     // Criar QUEUES e THREADS
    for(i = 0; i < NUMBER_OF_PROXIES; i++){

      threads[i].requests_bucket = requests_bucket; // Cada THREAD PROXY terá acesso à tabela de mensagens,
      threads[i].bucket_has_requests = &bucket_has_requests; // bem como a esta variável de estado,
      threads[i].monitor_bucket_has_requests = &monitor_bucket_has_requests; // a este monitor,
      threads[i].bucket_access = &bucket_access;  // e ao MUTEX para acesso à tabela.
      // Identificar o TABLE_SERVER a que cada PROXY se ligará
      threads[i].server_address_and_port = system_rtables[i+1];
      threads[i].id = i+1; // SWITCH com id 0, PROXIES com id's >= 1
      //threads[i].is_available = YES;

      // Criar cada uma das threads que serão PROXY de um TABLE_SERVER
      if (pthread_create(&thread_ids[i], NULL, &run_server_proxy, (void *) &threads[i]) != 0){
        perror("Erro ao criar uma thread proxy");
        return -1;
      }
      pthread_detach(thread_ids[i]);
    }

    
    

    /** Sets up all the poll structures to support multiple client connections **/


    //sockaddres_in struct for a client
    struct sockaddr_in client;
    //the connection socket with a client
    int connection_socket_fd;
    //the cliente socket size
    socklen_t  client_socket_size = sizeof (client);
   
    /* initializes the table_skel */
    if ( table_skel_init_with_mode( N_TABLE_SLOTS, SWITCH_RESPONSE_MODE ) == TASK_FAILED )
        return TASK_FAILED;



    /** creates a pollfd **/ 
    struct pollfd connections[N_MAX_CLIENTS];
    /* the socket_fd is the listening socket */
    connections[0].fd = socket_fd;
    connections[0].events = POLLIN;
    /* initializes each connection slot */
    for( i=1; i < N_MAX_CLIENTS; i++){
        connections[i].fd = -1;
        connections[i].events = 0;
        connections[i].revents = 0;
    }

    //for now, only the listening socket
    int connected_fds = 1;
    //saves the highet connection index
    int highestIndexConnection = 0;
    // to save the result from poll function
    int polled_fds = 0; 


    printf("\n--------- waiting for clients requests ---------\n");


    while ((polled_fds = poll(connections, connected_fds, 50)) >= 0) {

        //if there was any polled sockets fd with events
        if ( polled_fds > 0 ) { 

            /** enters if there is a request on the listening socket **/ 
            if ( (connections[0].revents & POLLIN) && (connected_fds < N_MAX_CLIENTS) ) {  
                /* gets an open slot*/
                int open_slot = get_open_slot(connections, connected_fds); 

                if ((connections[open_slot].fd = accept(connections[0].fd, (struct sockaddr *) &client, &client_socket_size)) > 0){ // Ligação feita ?
                    connections[open_slot].events = POLLIN; // Vamos esperar dados nesta socket
                    connected_fds++;
                    
                    //updates the highest index connection if needed.
                    if ( open_slot > highestIndexConnection ) {
                        highestIndexConnection = open_slot;
                    }
                }
            }

            //for each connected cliente it will receive a request and give a response
            int i = 0;
            for (i = 1; i <= highestIndexConnection; i++) {
                
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

                    /** Gets the client request as it was**/
                    struct message_t * client_request_raw = server_receive_request(connection_socket_fd);
                    /** prepares the raw client request to respect the authority of the switch **/
                    struct message_t * client_request = request_to_switch_mode(client_request_raw);
                    //checks error
                    failed_tasks += client_request == NULL;


                    /** If client request is a writter it's proxies work, otherwise will send report  **/

                    if ( message_is_writer(client_request) ) {

                        /** UPDATES THE REQUESTS_BUCKET **/

                        /* locks the access to the bucket */
                        pthread_mutex_lock(&bucket_access); 

                        /** If bucket is not full it will store the client_request **/
                        if ( !bucket_is_full ) { 

                            // Alocar memória para uma mensagem local (entre thread principal e as outras)
                            if ((current_request = (struct request_t *) malloc(sizeof(struct request_t))) == NULL) {
                                perror("Erro ao alocar memória para mensagem local");
                                return -1;
                            }

                            // Preparar mensagem local de acordo com o pedido do cliente
                            current_request->requestor_fd = connection_socket_fd; 
                            current_request->request = client_request; 
                            current_request->response = NULL;
                            current_request->flags = 5; 
                            current_request->acknowledged = NUMBER_OF_PROXIES;
                            current_request->deliveries = 0;
                            current_request->answered = NO;   
                            //current_request->id = total_requests_count;     

                            // Colocar mensagem na tabela
                            requests_bucket[index_to_store_request] = current_request; 
                            // Próxima mensagem será escrita neste índice
                            index_to_store_request = (index_to_store_request+1) % REQUESTS_BUCKET_SIZE;         
                            // Incrementar número de mensagens no bucket       
                            requests_counter++;  
                            //increments the number of requests ever received
                            total_requests_count++;


                            // Forçar este estado
                            bucket_has_requests = YES;           
                            // Sinalizar THREADS bloqueadas no estado vazio da tabela
                            monitor_signal(&monitor_bucket_has_requests, &bucket_has_requests); 
                            /* bucket is full if the slot of the next index is not empty */
                            bucket_is_full = requests_bucket[index_to_store_request] != NULL;   
                        }

                        /* unlocks the bucket */
                        pthread_mutex_unlock(&bucket_access); 
                    }
                    else {
                        /** else > client_request is a reader operation so will send it a report **/

                        struct message_t *server_response = message_create_with(OC_REPORT, CT_INVCMD, "Invalid command to switch.");
                        //sends the response to the client
                        int message_was_sent = server_send_response(connection_socket_fd, 1, &server_response);
                        //error case
                        failed_tasks+= message_was_sent == TASK_FAILED;

                        /** IF some error happened, it will notify the client **/
                        if ( failed_tasks > 0 ) {
                            server_sends_error_msg(connection_socket_fd);
                        }

                    }

                    sleep(4);

                    /** TIME TO CHECK FOR REPLIED REQUESTS FROM PROXIES **/
                    for ( i = 0; i < REQUESTS_BUCKET_SIZE; i++ ) {

                        //resets the flag value for each request 
                        failed_tasks = 0;

                        /* locks the access to the bucket */
                        pthread_mutex_lock(&bucket_access); 

                        
                        /** checks if the request i exists and got a response already **/
                        if (requests_bucket[i] != NULL && requests_bucket[i]->response != NULL) { 
                        
                            /* checks if there is the request i wasn't answered to the requestor yet */
                            if (!requests_bucket[i]->answered){  

                                /** where all the response message will be stored **/
                                struct message_t ** response_messages = NULL;

                                /** it will only invoke the request on itself if it went well on the other servers **/
                                if ( response_with_success(requests_bucket[i]->request, requests_bucket[i]->response) ) {

                                    int response_messages_num = invoke(requests_bucket[i]->request, &response_messages);
                                    // error case
                                    failed_tasks+= response_messages_num <= 0 || response_messages == NULL;

                                    //sends the response to the client
                                    int message_was_sent = server_send_response(requests_bucket[i]->requestor_fd, response_messages_num, response_messages);
                                    //error case
                                    failed_tasks+= message_was_sent == TASK_FAILED;
                                }
                                else {
                                    //declares that there was (at least) one failed that task once he 
                                    //response from the proxie was not a success response to the request.
                                    failed_tasks = 1;
                                } 
                               
                                /** IF some error happened, it will notify the client **/
                                if ( failed_tasks > 0 ) {
                                    server_sends_error_msg(requests_bucket[i]->requestor_fd);
                                }


                                /* if it went well it assumes if will also go well with all other proxies and 
                                   if went wrong we assume it also will with all other proxies,
                                   and an error message was sent so the work for this request is done*/
                                requests_bucket[i]->answered = YES; 
                                
                            }

                            /* if the request was acknowledge by every proxy it can now be removed from the bucket */
                            if (requests_bucket[i]->acknowledged <= 0) {
                                puts("\t--- sent to all servers so will be removed.");
                                free(requests_bucket[i]->response);
                                free(requests_bucket[i]->request);
                                free(requests_bucket[i]);
                                requests_bucket[i] = NULL; 
                                bucket_is_full = NO;    
                                requests_counter--; 
                                bucket_has_requests = requests_counter > 0;
                            }
                        }
                        /* by last, it unlocks the access to the bucket */
                        pthread_mutex_unlock(&bucket_access);
             
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

    char * my_address_and_port = strdup(argv[1]);

     /** gets the address_and_port of each remote table of the system **/
    char** system_rtables = NULL;
    int numberOfServers = get_system_rtables_info(SYSTEM_CONFIGURATION_FILE,  &system_rtables);

    /* checks if its the switch */
    int switchIAM = strcmp(system_rtables[0], my_address_and_port) == 0;


    if ( switchIAM ) {
        puts("\n\t ### I AM THE SWITCH ###");
        switch_run(my_address_and_port, system_rtables, numberOfServers);
    }
    else {
        puts("\n\t ### I AM A SERVER ###");
        server_run(my_address_and_port, system_rtables, numberOfServers);
    }
    


    return TASK_SUCCEEDED;
}