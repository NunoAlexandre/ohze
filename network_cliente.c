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
#include "define-utils.h"

/* Esta função deve:
 * - estabelecer a ligação com o servidor;
 * - address_port é uma string no formato <hostname>:<port>
 * (exemplo: 10.10.10.10:10000)
 * - retornar toda a informacão necessária (e.g., descritor da
 * socket) na estrutura server_t
 */
struct server_t *network_connect(const char *address_port){
    char *host_address;
	char *port;
	char *address_port_copy = strdup(address_port);
	struct sockaddr_in server;
    
    //finding host address and port
    host_address = strtok(address_port_copy, ":"); //get the host address
    port = strtok(NULL,":"); //get port
    
    //building struct server_t server_to_connect
    struct server_t *server_to_connect = (struct server_t*) malloc(sizeof(struct server_t));
    server_to_connect->ip_address = host_address;
    server_to_connect->port = atoi(port);
    
    /*---- Create the TCP socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) TCP protocol (0) */
    if((server_to_connect->socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("ERROR while creating TCP socket!");
        //returning to starting state
        free(address_port_copy);
        free(server_to_connect);
        return NULL; //será?
    }
    
    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    server.sin_family = AF_INET;
    
    /* Set port number, using htons function to use proper byte order */
    server.sin_port = htons(server_to_connect->port);
    
    /*converts host address to network format*/
    if (inet_pton(AF_INET, server_to_connect->ip_address, &server.sin_addr) < 1){
        perror("ERROR while converting Host address (IP) to network address structure.");
        //returning to starting state
        close(server_to_connect->socketfd);
        free(address_port_copy);
        free(server_to_connect);
        return NULL;
    }
    
    /*---- Connect the socket to the server using the address struct ----*/
    //initiates the connection with server
    if (connect(server_to_connect->socketfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror ("ERROR while connecting with server!");
        //returning to starting state
        close(server_to_connect->socketfd);
        free(address_port_copy);
        free(server_to_connect);
        return NULL;
    }
    
    free(address_port_copy);
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
    if (receive_message(server->socketfd) == NULL){
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
