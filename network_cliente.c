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
    
    puts("Connecting to server...");

    retry_connection = YES;
    
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
        
        perror ("NETWORK_CLIENT --> NETWORK_CONNECT > ERROR while connecting with server!");

        //tries to reconnect to server if ECONNREFUSED occurred and retry_connection = YES
        if (network_retransmit(server_to_connect->socketfd)){
            puts("NETWORK_CLIENT --> NETWORK_CONNECT > Trying to reconnect to server...");
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
        close(server_to_connect->socketfd);
        puts("did shutdown the socket!!!!!!!!!!!!");
        free(server_to_connect);
        return NULL;
        }
    }
    
    /*returns the server that we want to conect with*/
    puts("Connected to server...");
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
        perror("NETWORK_CLIENT --> NETWORK_SEND_RECEIVE > FAILED TO SEND MESSAGE!");
        
        //tries to reconnect to server if ECONNREFUSED occurred and retry_connection = YES
        if (network_retransmit(server->socketfd)){
            struct server_t * reconnected_server;
            sleep(RETRY_TIME);
            network_close(server); //fecha ligação
            reconnected_server = network_reconnect(server); //faz um reconnect
            
            //verifica network_reconnect
            if (reconnected_server == NULL) return NULL;
            
            server = reconnected_server;
            puts ("NETWORK_CLIENT --> NETWORK_SEND_RECEIVE > TRYING TO RESEND MESSAGE");
            
            //verifica menssagem (re)enviada
            if(send_message(server->socketfd, msg) == TASK_FAILED){
                perror("NETWORK_CLIENT --> NETWORK_SEND_RECEIVE > FAILED TO RESEND MESSAGE!");
                return NULL;
            }
        }
        
        else return NULL; //correu mal e não houve nova tentativa

        
    };
    
    /* 2. Criação das estruturas para receber mensagem */
    struct message_t* received_msg = NULL;
    
    /* 3. Recebe mensagem do servidor com base no socketfd */
    received_msg = receive_message(server->socketfd);
    
    /* 4. Validação da mensagem recebida */
    if (received_msg == NULL){
        perror("NETWORK_CLIENT --> NETWORK_SEND_RECEIVE > FAILED TO RECEIVE MESSAGE!");
        
        //tries to reconnect to server if ECONNREFUSED occurred and retry_connection = YES
        if (network_retransmit(server->socketfd)){
            struct server_t * reconnected_server;
            sleep(RETRY_TIME);
            network_close(server); //fecha ligação
            reconnected_server = network_reconnect(server); //faz um reconnect
            
            //verifica network_reconnect
            if (reconnected_server == NULL) return NULL;
            
            server = reconnected_server;
            
            puts("NETWORK_CLIENT --> NETWORK_SEND_RECEIVE > LAST TRY TO RECEIVE MESSAGE!");
            received_msg = receive_message(server->socketfd);
            
            //verifica menssagem (re)recebida
            if (received_msg == NULL){
                perror("NETWORK_CLIENT --> NETWORK_SEND_RECEIVE > LAST TRY TO RECEIVE MESSAGE FAILLED!");
                return NULL;
            }
            return received_msg; //correu tudo bem com a tentiva de voltar a receber msg
            
        }
        return NULL; //correu mal e não houve nova tentativa
    }
    
    return received_msg; //correu tudo bem
}

/* A funcao network_close() deve fechar a ligação estabelecida por
 * network_connect(). Se network_connect() alocou memoria, a função
 * deve libertar essa memoria.
 */
int network_close(struct server_t *server){
    int task = TASK_SUCCEEDED;
    shutdown(server->socketfd, SHUT_RDWR);
    task = close(server->socketfd);

    if (task == TASK_FAILED){
        perror("NETWORK_CLIENT --> NETWORK_CLOSE > FAILED TO CLOSE CONNECTION!");
        return TASK_FAILED;
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
        perror("NETWORK_CLIENT --> NETWORK_RECONNECT > ERROR while reconnecting with server!");
        //returning to starting state
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
    
    printf ("server->reconnect_to_server: %d\n",retry_connection);
    printf ("errno: %d\n",errno);

    if (retry_connection == YES && errno == EPIPE){
        puts ("É para retransmitir!");
        retry_connection = NO;
        return YES;
    }
    
    else if (retry_connection == YES && (errno == ECONNREFUSED) ) {
        puts ("É para reconnectar!");
        retry_connection = NO;
        return YES;
    }
    else if (retry_connection == YES && (!socket_is_open(socket_fd)) )  {
        puts ("É para reconnectar!");
        retry_connection = NO;
        return YES;
    }    
    else{
        puts("NADA A FAZER -- JÁ FOI FEITO UM RETRANSMIT!");
        retry_connection = NO;
        return NO;
    }
}