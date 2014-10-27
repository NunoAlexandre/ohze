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



int server_send_result (int connection_socket_fd, int opcode, int content_type, int value) {
    struct message_t * messageToSend = message_create_with_result(opcode, content_type, value);
    
    //buffer message
    char ** messageToSend_buffer;
    int messageToSend_size = message_to_buffer(messageToSend, messageToSend_buffer);
    
    if ( messageToSend_size <= 0 || messageToSend_size > MAX_MSG) {
        return TASK_FAILED;
    }
    
    int message_buffer_size_n = htonl(messageToSend_size);
    
    //sends the size of the message
    write(connection_socket_fd, &message_buffer_size_n, BUFFER_INTEGER_SIZE);
    //and sends the message
    write(connection_socket_fd, *messageToSend_buffer, strlen(*messageToSend_buffer));
    
    return TASK_SUCCEEDED;
}

/*
 * Sends a tupleToSend to socket connection_socket_fd.
 * Firstly it sends one integer with the size of the message and then it sends the buffer message.
 */
int server_send_tuple (int connection_socket_fd, int opcode, struct tuple_t * tupleToSend ) {
    
    //creates the message that will be sent.
    struct message_t * messageToSend = message_create_with_tuple(opcode+1, CT_TUPLE, tupleToSend);
    if ( messageToSend == NULL)
        return TASK_FAILED;
    
    //creates the message buffer to send to the cliente
    char ** messageToSend_buffer;
    int message_size_n = htonl(message_to_buffer(messageToSend, messageToSend_buffer));
    
    if ( message_size_n <= 0 || message_size_n > MAX_MSG)
        return TASK_FAILED;
    
    //sends the size of the message
    write(connection_socket_fd, &message_size_n, BUFFER_INTEGER_SIZE);
    //and sends the message
    write(connection_socket_fd, *messageToSend_buffer, strlen(*messageToSend_buffer));
    
    return TASK_SUCCEEDED;
}

int server_send_tuples (int connection_socket_fd, int opcode, struct list_t * matching_nodes) {
    
    //1. sends a message to the client letting him now how many nodes it will receive
    // if there are no tuples to send it sends "0/zero" to the cliente and stops here.
    int nodes_to_receive_n = htonl(list_size(matching_nodes));
    write(connection_socket_fd, &nodes_to_receive_n, BUFFER_INTEGER_SIZE);
    
    
    //then, for each tuple of the list it will send it
    int tuplesToSend = list_size(matching_nodes);
    struct node_t * currentNode = list_head(matching_nodes);
    
    while ( tuplesToSend-- > 0 ) {
        //sends the tuple
        server_send_tuple(connection_socket_fd, opcode, entry_value(node_entry(currentNode)));
        //moves the node pointer
        currentNode = currentNode->next;
    }
    
    return TASK_SUCCEEDED;
}

int server_put (int connection_socket_fd, struct table_t * server, struct message_t * client_msg ) {
    
    //safety check
    if ( server == NULL || client_msg->content.tuple == NULL )
        return TASK_FAILED;
    
    struct tuple_t * tuple_to_add = client_msg->content.tuple;
    
    //if both tasks perform successefully taskSuccess will keep steady.
    int taskSuccess = TASK_SUCCEEDED;
    taskSuccess = table_put(server,  tuple_to_add);
    taskSuccess+= server_send_result(connection_socket_fd, client_msg->opcode+1, client_msg->c_type, taskSuccess );
    
    return taskSuccess == TASK_SUCCEEDED ? TASK_SUCCEEDED : TASK_FAILED;
}

/*
 * Gets one_or_all matching node(s) from the server table and sends it/them to the cliente.
 *
 */
int server_get_send_tuples ( int connection_socket_fd, table_t * server, struct message_t * cliente_request, int one_or_all) {
    
    //flag to give task return
    int taskSuccess = TASK_FAILED;
    
    // first it has to know what must happen to the matching nodes
    int whatToDoWithTheNode = cliente_request->opcode == OC_IN ? DONT_KEEP_AT_ORIGIN : KEEP_AT_ORIGIN;
    
    // gets the matching nodes
    struct list_t * matching_nodes = table_get(server, cliente_request->content.tuple, whatToDoWithTheNode, one_or_all);
    
    if ( one_or_all == 1 ) {
        struct tuple_t * matched_tuple = list_isEmpty(matching_nodes) ? NULL : entry_value(node_entry((list_head(matching_nodes))));
        taskSuccess = server_send_tuple(connection_socket_fd, cliente_request->opcode, matched_tuple);
    }
    else {
        //sends all the matching nodes
        taskSuccess = server_send_tuples(connection_socket_fd, cliente_request->opcode, matching_nodes);
    }
    
    //destroyes the list matching nodes to free memory
    if ( ! list_isEmpty(matching_nodes) )
        list_destroy(matching_nodes);
    
    //returns the success of sending the tuple to the cliente.
    return taskSuccess;
}

int server_send_table_size ( int connection_socket_fd, table_t * server ) {
    
    int server_table_size = table_size(server);
    int taskSuccess = server_send_result(connection_socket_fd, OC_SIZE+1, CT_RESULT, server_table_size);
    
    return taskSuccess;
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
            
            //all the cases that involve get elements from the server table
        case OC_IN:
        case OC_COPY :
        case OC_IN_ALL:
        case OC_COPY_ALL :
        {
            int one_or_all = cliente_request->opcode == OC_IN || cliente_request->opcode == OC_COPY;
            taskSuccess = server_get_send_tuples(connection_socket_fd, server, cliente_request, one_or_all );
            break;
        }
            
            //inserir um tuplo à tabela
        case OC_OUT:
        {
            taskSuccess = server_put(connection_socket_fd, server, cliente_request);
            //sends a message back saying if it was added
            break;
        }
            //devolve o numero de elements da tabela
        case OC_SIZE:
        {
            taskSuccess = server_send_table_size(connection_socket_fd, server);
            //does what it should
            break;
        }
            
        default:
        {
            printf("Client asked a wrong operation\n");
            taskSuccess = TASK_FAILED;
            break;
        }
    }
    return taskSuccess;
}

int server_sends_error_msg( int connection_socket_fd ) {
    while( server_send_result(connection_socket_fd, OP_ERROR, CT_RESULT, TASK_FAILED) != TASK_FAILED) ;
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
    
    //flag
    int taskSuccess = TASK_FAILED;
    //where the request message will be stored
    char * request_msg [MAX_MSG];
    //number of bytes of the request message
    long request_msg_size = 0;
    
    //gets the request_msg_size that will be received after this
    if ((request_msg_size = read(connection_socket_fd,&request_msg_size, BUFFER_INTEGER_SIZE)) < 0) {
        perror("Erro ao receber dados do cliente");
        close(connection_socket_fd);
        return taskSuccess;
    }
    //converts to host format
    request_msg_size = ntohl(request_msg_size);
    
    //reads a message from the client
    if( read(connection_socket_fd,request_msg, request_msg_size) != request_msg_size ) {
        perror("Erro ao receber dados do cliente");
        close(connection_socket_fd);
        return taskSuccess;
    }
    
    // sets the end of the msg
    request_msg[request_msg_size] = '\0';
    
    //converts the serialized message to a struct message_t
    struct message_t * cliente_request = buffer_to_message(*request_msg, (int)request_msg_size);
    
    
    /**** Sends the proper response to the cliente  ****/
    taskSuccess = send_response(server_table, connection_socket_fd, cliente_request);
    
    //if something went wrong it sends an error msg
    if ( taskSuccess == TASK_FAILED ) {
        server_sends_error_msg(connection_socket_fd);
    }
    
    // Fecha socket referente a esta conexao
    close(connection_socket_fd);
    
    return taskSuccess;
}



/* A funcao network_close() deve fechar a ligação estabelecida por
 * network_connect(). Se network_connect() alocou memoria, a função
 * deve libertar essa memoria.
 */
int network_close(int socketfd );