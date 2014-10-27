//
//  network_server.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 26/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include "inet.h"
#include "table-private.h"
#include "message-private.h"



/*
 * Recebe um socket fd de comunicação e uma messagem que contem
 * um pedido por um cliente e processa o pedido e
 * age em conformidade, enviando uma resposta de volta.
 */
int send_response (int connection_socket_fd, struct message_t *cliente_request) {
    
    // Tem de fazer um switch case para para caso de mensagem
    // Dependendo do tipo de mensagem a enviar.
    switch ( cliente_request->opcode ) {
            
        //inserir um tuplo à tabela
        case OC_OUT :
            //does what it should
            break;
        
        //devolve um matching tuple
        case OC_IN :
            //does what it should
            break;
            
        //devolve uma sequencia de matching tuplos
        case OC_IN_ALL :
            //does what it should
            break;
            
        //devolve a copia de um matching tuple
        case OC_COPY :
            //does what it should
            break;

        //devolve uma sequencia de matching tuplos copiados
        case OC_COPY_ALL :
            //does what it should
            break;
            
        //devolve o numero de elements da tabela
        case OC_SIZE:
            //does what it should
            break;

            
        default:
            break;
    }
    
    
    return TASK_SUCCEEDED;
    
}

/* Esta função é a correspondente à send_receive do network_client.
 * - Obter o descritor da ligação (socket) da estrutura server_t;
 * - enviar a mensagem msg ao servidor;
 * - receber uma resposta do servidor;
 * - retornar a mensagem obtida como resposta ou NULL em caso
 * de erro.
 */
int network_receive_send(int connection_socket_fd ) {

    //where the request message will be stored
    char * request_msg [MAX_MSG];
    //number of bytes of the request message
    int request_msg_size = 0;
    
    /*
     reads a request from the cliente
     */
    if((request_msg_size = read(connection_socket_fd,request_msg, MAX_MSG)) < 0){
        perror("Erro ao receber dados do cliente");
        close(connection_socket_fd);
        return TASK_FAILED;
    }
    
    // sets the end of the msg
    request_msg[request_msg_size] = '\0';
    
    //converts the serialized message to a struct message_t
    struct message_t * cliente_request = buffer_to_message(*request_msg, request_msg_size);
  
    
    if ( send_response(connection_socket_fd, cliente_request) == TASK_FAILED ) {
        return TASK_FAILED;
    }
    
    // Fecha socket referente a esta conex„o
    close(connection_socket_fd);
    
    
    return TASK_SUCCEEDED;
}



/* A funcao network_close() deve fechar a ligação estabelecida por
 * network_connect(). Se network_connect() alocou memoria, a função
 * deve libertar essa memoria.
 */
int network_close(struct server_t *server);

