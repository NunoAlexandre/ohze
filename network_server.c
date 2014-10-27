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
#include "message.h"
#include "message-private.h"
#include "tuple-private.h"
#include "tuple.h"



int write_all ( int connection_socket_fd, int operation_code, int content_type ) {

    int messagesToSend = 0;
    
    
    
    return TASK_SUCCEEDED;
}

/*
 * Sends a tupleToSend to socket connection_socket_fd.
 * Firstly it sends one integer with the size of the message and then it sends the buffer message.
 */
int server_send_tuple (int connection_socket_fd, int opcode, struct tuple_t * tupleToSend ) {
    
    //creates the message that will be sent.
    struct message_t * messageToSend = message_create_with_tuple(opcode+1, CT_TUPLE, tupleToSend);
    //creates the message buffer to send to the cliente
    char ** messageToSend_ready;
    message_to_buffer(messageToSend, messageToSend_ready);
    int message_size = htonl(message_size_bytes(messageToSend));
    //sends the size of the message
    write(connection_socket_fd, &message_size, RESULT_SIZE);
    //and sends the message
    write(connection_socket_fd, *messageToSend_ready, strlen(*messageToSend_ready));
    
    return TASK_SUCCEEDED;
}

/*
 * Gets a matching node from the server table and sends it to the cliente.
 *
 */
int server_get_send_tuple ( int connection_socket_fd, table_t * server, struct message_t * cliente_request ) {
    
    int whatToDoWithTheNode = cliente_request->opcode == OC_IN ? DONT_KEEP_AT_ORIGIN : KEEP_AT_ORIGIN;
    
    struct list_t * list_matching_node = table_get(server, cliente_request->content.tuple, whatToDoWithTheNode, 1);
    
    struct tuple_t * matched_tuple = entry_value(node_entry((list_head(list_matching_node))));
    
    //returns the success of sending the tuple to the cliente.
    return server_send_tuple(connection_socket_fd, cliente_request->opcode, matched_tuple);
}



/*
 * Recebe um socket fd de comunicação e uma messagem que contem
 * um pedido por um cliente e processa o pedido e
 * age em conformidade, enviando uma resposta de volta.
 */
int send_response (struct table_t * server, int connection_socket_fd, struct message_t *cliente_request) {
    
    //flag to return in the end
    int taskSuccess = TASK_FAILED;
    
    // Tem de fazer um switch case para para caso de mensagem
    // Dependendo do tipo de mensagem a enviar.
    switch ( cliente_request->opcode ) {
            
        //inserir um tuplo à tabela
        case OC_OUT:
        {
            struct tuple_t * tuple_to_add = cliente_request->content.tuple;
            taskSuccess = table_put(server,  tuple_to_add);
            //sends a message back saying if it was added
            break;
        }
        //devolve um matching tuple
        case OC_IN:
        case OC_COPY :
        {
            taskSuccess = server_get_send_tuple(connection_socket_fd, server, cliente_request);
            break;
        }
        //devolve uma sequencia de matching tuplos
        case OC_IN_ALL :
        case OC_COPY_ALL  :
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
int network_receive_send(table_t * server_table,  int connection_socket_fd ) {

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
  
    
    if ( send_response(server_table, connection_socket_fd, cliente_request) == TASK_FAILED ) {
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
int network_close(int socketfd );

