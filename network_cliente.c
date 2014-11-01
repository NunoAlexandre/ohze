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
    
    //1. get server_address and server_port
    char * server_address = get_address(address_port);
    //if server_address is a hostname converts to IP, if alredy IP keeps the same.
    char server_address_ip[200];
    int result = hostname_to_ip(server_address, server_address_ip);
    if ( result == TASK_FAILED ) return NULL;
    char * server_port =  get_port(address_port);
 
    //2.building struct server_t server_to_connect
    struct server_t *server_to_connect = (struct server_t*) malloc(sizeof(struct server_t));
    server_to_connect->ip_address = server_address_ip;
    server_to_connect->port = atoi(server_port);


    // Create the TCP socket with 1) Internet domain 2) Stream socket 3) TCP protocol (0)
    if((server_to_connect->socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("ERROR while creating TCP socket!");
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
        perror("ERROR while converting Host address (IP) to network address structure.");
        //returning to starting state
        close(server_to_connect->socketfd);
        free(server_to_connect);
        return NULL;
    }
    
    /*---- Connect the socket to the server using the address struct ----*/
    //initiates the connection with server
    if (connect(server_to_connect->socketfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror ("ERROR while connecting with server!");
        //returning to starting state
        close(server_to_connect->socketfd);
        free(server_to_connect);
        return NULL;
    }
    
    /*returns the server that we want to conect with*/
    return server_to_connect;
}

/* Esta função deve
 * - Obter o descritor da ligação (socket) da estrutura server_t;
 * - enviar a mensagem msg ao servidor;
 * - receber uma resposta do servidor;
 * - retornar a mensagem obtida como resposta ou NULL em caso
 * de erro.
 */
struct message_t *network_send_receive(struct server_t *server, struct message_t *msg){
    
    /* 1. Envia mensagem para servidor com base no socketfd */
    if (send_message(server->socketfd, msg) == TASK_FAILED){
        perror("NETWORK_CLIENT --> FAILED TO SEND MESSAGE!");
        return NULL;
    };
    
    /* 2. Criação das estruturas para receber mensagem */
    struct message_t* received_msg;
    
    /* 3. Recebe mensagem do servidor com base no socketfd */
    received_msg = receive_message(server->socketfd);
    /* 4. Validação da mensagem recebida */
    if (received_msg == NULL){
        perror("NETWORK_CLIENT --> FAILED TO RECEIVE MESSAGE!");
        return NULL;
    }
    
    return received_msg;
}

/* A funcao network_close() deve fechar a ligação estabelecida por
 * network_connect(). Se network_connect() alocou memoria, a função
 * deve libertar essa memoria.
 */
int network_close(struct server_t *server){
    int task = 0;
    task = close (server->socketfd);
    if (task == -1){
        return -1;
    }
    free (server->ip_address);
    free (server);
    return task;
}
