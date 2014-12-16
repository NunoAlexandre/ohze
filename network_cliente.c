//
//  network_cliente.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 25/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include "inet.h"
#include "network_cliente.h"
#include "network_utils.h"
#include "network_client-private.h"
#include "general_utils.h"

/* Esta função deve:
 * - estabelecer a ligação com o servidor;
 * - address_port é uma string no formato <hostname>:<port>
 * (exemplo: 10.10.10.10:10000)
 * - retornar toda a informacão necessária (e.g., descritor da
 * socket) na estrutura server_t
 */
struct server_t *network_connect(const char *address_port) {
    
    puts("--- connecting to server...");
    
    retry_connection = YES;
    
    //1. get server_address and server_port
    char * server_address = get_address(address_port);
    //if server_address is a hostname converts to IP, if alredy IP keeps the same.
    char server_address_ip[200];
    int result = hostname_to_ip(server_address, server_address_ip);
    if ( result == FAILED ) return NULL;
    char * server_port =  get_port(address_port);
    
    //2.building struct server_t server_to_connect
    struct server_t *server_to_connect = (struct server_t*) malloc(sizeof(struct server_t));
    server_to_connect->ip_address = server_address;
    server_to_connect->port = atoi(server_port);
    
    
    // Create the TCP socket with 1) Internet domain 2) Stream socket 3) TCP protocol (0)
    if((server_to_connect->socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("NETWORK_CLIENT --> NETWORK_CONNECT > ERROR while creating TCP socket!");
        //returning to starting state
        free(server_to_connect);
        return NULL;
    }
    
    
    struct sockaddr_in server;
    //Configure settings of the server address struct
    server.sin_family = AF_INET;
    // Set port number, using htons function to use proper byte order
    server.sin_port = htons(server_to_connect->port);
    
    //converts host address to network format
    if (inet_pton(AF_INET, server_to_connect->ip_address, &server.sin_addr) < 1){
        perror("NETWORK_CLIENT --> NETWORK_CONNECT > ERROR while converting Host address (IP) to network address structure.");
        //returning to starting state
        close(server_to_connect->socketfd);
        free(server_to_connect);
        return NULL;
    }
    
    /*---- Connect the socket to the server using the address struct ----*/
    //initiates the connection with server
    int connection = connect(server_to_connect->socketfd, (struct sockaddr *) &server, sizeof(server));
    
    //coloca em memória o endereço ip!
    ip_address_copy_from_server = strdup(server_to_connect->ip_address);
    
    if (connection < 0){
        
        int reconnected = NO;
        
        perror ("\t--- error while connecting to the server");
        
        //tries to reconnect to server if ECONNREFUSED occurred and retry_connection = YES
        if (network_retransmit(server_to_connect->socketfd)){
            puts("\t--- trying to reconnect to server...");
            sleep(RETRY_TIME);
            struct server_t *server_reconnected;
            
            server_reconnected = network_reconnect(server_to_connect);
            
            if (server_reconnected == NULL) return NULL; //network_reconnect FAILED!
            reconnected = YES;
            server_to_connect = server_reconnected;
        }
        
        //(re)connection fails and returns to initial state
        if (connection < 0 && reconnected == NO){
            //returning to starting state
            
            shutdown(server_to_connect->socketfd, SHUT_RDWR);
            puts("\t--- did shutdown the socket.");
            if ( server_to_connect != NULL)
                free(server_to_connect);

            return NULL;
        }
    }
    
    /*returns the server that we want to conect with*/
    puts("--- connected to server...");
    return server_to_connect;
}

void network_reset_retransmissions() {
    retry_connection = YES;
}

/* Esta função deve
 * - Obter o descritor da ligação (socket) da estrutura server_t;
 * - enviar a mensagem msg ao servidor;
 * - receber uma resposta do servidor;
 * - retornar a mensagem obtida como resposta ou NULL em caso
 * de erro.
 */
struct message_t * network_send_receive(struct server_t *server, struct message_t *msg){
    
    network_reset_retransmissions();
    
    int taskSucceeded = NO;
    int retries = 0;
    struct message_t* received_msg = NULL;
    
    while ( retries <= 1 && !taskSucceeded ) {
        if (send_message(server->socketfd, msg) == SUCCEEDED){
            usleep(200);
            received_msg = receive_message(server->socketfd);
            taskSucceeded = received_msg != NULL;
        }
        
        if ( ! taskSucceeded ) {
            if (network_retransmit(server->socketfd)){
                sleep(RETRY_TIME);
                network_close(server); //fecha ligação
                server = network_reconnect(server); //faz um reconnect
                retries = server == NULL ? 2 : retries;
            }
        }
        retries++;
    }
    
    return taskSucceeded ? received_msg : NULL;
}

/* A funcao network_close() deve fechar a ligação estabelecida por
 * network_connect(). Se network_connect() alocou memoria, a função
 * deve libertar essa memoria.
 */
int network_close(struct server_t *server){
    int task = SUCCEEDED;
    shutdown(server->socketfd, SHUT_RDWR);
    task = close(server->socketfd);
    
    if (task == FAILED){
        return FAILED;
    }
    return task;
}


/*
 * If something happens with the connection, the client tries to
 * reconnect with server using the same mechanisms that network_connect function
 * returns server to reconnect
 */
struct server_t *network_reconnect(struct server_t *server_to_connect){
    
    struct server_t *server_to_reconnect = (struct server_t*) malloc(sizeof(struct server_t));
    server_to_reconnect->ip_address = strdup (ip_address_copy_from_server);
    server_to_reconnect->port = server_to_connect->port;
    server_to_reconnect->socketfd = server_to_connect->socketfd;
    
    // 1. Creates the TCP socket with 1) Internet domain 2) Stream socket 3) TCP protocol (0)
    if((server_to_reconnect->socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("NETWORK_CLIENT --> NETWORK_RECONNECT > ERROR while creating TCP socket!");
        free(server_to_reconnect->ip_address);
        free(server_to_reconnect);
        return NULL;
    }
    
    struct sockaddr_in server;
    //Configure settings of the server address struct
    server.sin_family = AF_INET;
    // Set port number, using htons function to use proper byte order
    server.sin_port = htons(server_to_reconnect->port);
    
    //converts host address to network format
    if (inet_pton(AF_INET, server_to_reconnect->ip_address, &server.sin_addr) < 1){
        perror("NETWORK_CLIENT --> NETWORK_RECONNECT > ERROR while converting Host address (IP) to network address structure!");
        //returning to starting state
        close(server_to_reconnect->socketfd);
        free(server_to_reconnect->ip_address);
        free(server_to_reconnect);
        return NULL;
    }
    
    /*---- Connect the socket to the server using the address struct ----*/
    //initiates the connection with server
    if (connect(server_to_reconnect->socketfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        puts("\t--- UPS: Failed to reconnect");
        //returning to sdtarting state
        close(server_to_reconnect->socketfd);
        free(server_to_reconnect->ip_address);
        free(server_to_reconnect);
        return NULL;
    }
    
    /*returns the server that we want to reconect with*/
    return server_to_reconnect;
}


/*
 * Função que decide se vai haver nova tentativa de ligação ou
 * reenvio de msg
 */
int network_retransmit (int socket_fd){
    
    if (retry_connection == YES ) {
        puts ("\t--- will try to reconect");
        retry_connection = NO;
        return YES;
    }
    else{
        puts ("\t--- already tried to retransmit so will quit.");
        retry_connection = NO;
        return NO;
    }
}