//
//  client_stub.c
//  SD15-Product
//
//  Created by Bruno Mata on 10/11/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "general_utils.h"
#include "client_stub.h"
#include "network_cliente.h"
#include "network_utils.h"
#include "client_stub.h"
#include "message.h"


/*
 * Verifica se o opcode é do tipo Getter
 */
int opcode_is_getter (int opcode ) {
    return opcode == OC_IN || opcode == OC_IN_ALL || opcode == OC_COPY || opcode == OC_COPY_ALL;
}

/*
 * Assume que a msg_response é uma mensagem de sucesso para com a msg_request.
 */
int client_decision_to_take (struct message_t * msg_request, struct message_t * msg_response ) {
    if ( opcode_is_getter(msg_request->opcode) && msg_response->content.result > 0 ) {
        return CLIENT_RECEIVE_TUPLES;
    }
    return CLIENT_PROCEED;
}

/*
 * Função que dada dois argumentos (keep_tuples, on_or_all) define o opcode
 * retorna -1 em caso de erro
 */
int assign_opcode(int keep_tuples, int one_or_all){
    int opcode = OC_DOESNT_EXIST;
    
    if (keep_tuples == NO && one_or_all == NO){
        opcode = OC_IN;
    }
    
    else if (keep_tuples == NO && one_or_all == YES){
        opcode = OC_IN_ALL;
    }
    
    else if (keep_tuples == YES && one_or_all == NO){
        opcode = OC_COPY;
    }
    
    else if (keep_tuples == YES && one_or_all == YES){
        opcode = OC_COPY_ALL;
    }
    
    else{
        return TASK_FAILED;
    }
    
    return opcode;
}

/*
 * Função que dada uma estrutura server_t cria uma rtable
 * retorna NULL em caso de erro
 */
struct rtable_t* rtable_create_from_server ( struct server_t *server_to_connect, char *server_address_and_port){
    struct rtable_t *new_rtable = (struct rtable_t*) malloc(sizeof(struct rtable_t));
    
    puts ("CLIENTE_STUB > RTABLE_CREATE_FROM_SERVER > Creating new rtable...");

    char* ip_from_server = strdup(server_to_connect->ip_address);
    int port_number_from_server = server_to_connect->port;
    int socketfd_from_server = server_to_connect->socketfd;
    
    if(new_rtable == NULL){
        perror ("CLIENTE_STUB > RTABLE_CREATE_FROM_SERVER > Unable to create new rtable!");
        free(new_rtable);
        free(ip_from_server);
        return NULL;
    }
    
    new_rtable->server_to_connect.ip_address = ip_from_server;
    free(ip_from_server);
    new_rtable->server_to_connect.port = port_number_from_server;
    new_rtable->server_to_connect.socketfd = socketfd_from_server;
    new_rtable->server_address_and_port = strdup(server_address_and_port);
    new_rtable->retry_connection = YES;
    
    puts ("CLIENTE_STUB > RTABLE_CREATE_FROM_SERVER > New rtable created...");

    return new_rtable;
}

/*
 * Função que dada uma estrutura rtable cria uma server_t
 * retorna NULL em caso de erro
 */
struct server_t* server_create_from_rtable ( struct rtable_t *remote_table){
   
    puts ("CLIENTE_STUB > SERVER_CREATE_FROM_RTABLE > Creating new server...");

    struct server_t *new_server = (struct server_t*) malloc(sizeof(struct server_t));
    char* ip_from_rtable = strdup(remote_table->server_to_connect.ip_address);
    int port_number_from_rtable = remote_table->server_to_connect.port;
    int socketfd_from_rtable = remote_table->server_to_connect.socketfd;
    
    if(new_server == NULL){
        perror ("CLIENTE_STUB > SERVER_CREATE_FROM_RTABLE > Unable to create new server!");
        free(new_server);
        free(ip_from_rtable);
        return NULL;
    }
    
    new_server->ip_address = ip_from_rtable;
//    free(ip_from_rtable);
    new_server->port = port_number_from_rtable;
    new_server->socketfd = socketfd_from_rtable;
    
    puts ("CLIENTE_STUB > SERVER_CREATE_FROM_RTABLE > New server created!");
  
    return new_server;
}

/* Função para estabelecer uma associação com uma tabela num servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtable_t *rtable_bind(const char *address_port){
    char* address_port_copy = strdup (address_port);
    struct server_t *server_to_conect;
    server_to_conect = network_connect(address_port_copy);
    
    puts("CLIENT-STUB > RTABLE_BIND > Connecting to server...");
    
    //verificação da ligação ao servidor
    if (server_to_conect == NULL){
        perror ("CLIENT_STUB > RTABLE_BIND > Unable to connect to server!");
        free(address_port_copy);
        free(server_to_conect);
        return NULL; // neste momento estará a NULL
    }
    
    //a criar uma estrutura rtable
    struct rtable_t *remote_table_to_connect = rtable_create_from_server(server_to_conect, address_port_copy);
    
//    free(address_port_copy);
//    free(server_to_conect);
    puts("CLIENT-STUB > RTABLE_BIND > Connected to server!");
    return remote_table_to_connect;
}

/* Fecha a ligação com o servidor, liberta toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtable_unbind(struct rtable_t *rtable){
    int task = TASK_FAILED;
    
    //cria server_t com base em rtable
    struct server_t *connected_server = server_create_from_rtable(rtable);
    
    //verifica se server_t foi bem criado
    if (connected_server == NULL) {
        perror ("CLIENT_STUB > RTABLE_UNBIND > Unable to disconnect from server!");
        free(connected_server);
        return TASK_FAILED;
    }
    
    puts("CLIENT-STUB > RTABLE_UNBIND > Disconnecting from server...");
    
    //faz um network_close ao servidor
    task = network_close(connected_server);
    if(task == TASK_FAILED){
        return TASK_FAILED;
    }
    
    puts("CLIENT-STUB > RTABLE_UNBIND > Disconnected from server...");
//    free(connected_server);
    return TASK_SUCCEEDED;
}

/* Função para adicionar um tuplo na tabela.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtable_out(struct rtable_t *rtable, struct tuple_t *tuple){
    
    struct tuple_t *tuple_to_send = tuple;
    
    //cria server_t com base em rtable
    struct server_t *connected_server = server_create_from_rtable(rtable);

    puts("CLIENT-STUB > RTABLE_OUT > Creating message...");

    //cria mensagem com OC_OUT, CT_TUPLE e tuple
    struct message_t *message_to_send = message_create_with(OC_OUT, CT_TUPLE, tuple_to_send);
    
    puts("CLIENT-STUB > RTABLE_OUT > Sending message to server...");
    
    //envia mensagem para o servidor e recebe mensagem do servidor com o resultado da operação
    struct message_t *received_message = network_send_receive(connected_server, message_to_send);

    puts("CLIENT-STUB > RTABLE_OUT > Received message from server!");
    
    //verifica se a mensagem recebida foi de sucesso
    if (response_with_success(message_to_send, received_message) == NO){
        puts("CLIENT-STUB > RTABLE_OUT > RECEIVED MESSAGE WITH ERROR OPCODE or OPCODE UNEXPECTED.");
        free(message_to_send);
        free(received_message);
        return TASK_FAILED;
    }
    

//    free (message_to_send);
//    free (received_message);
//    free (connected_server);
    
    return TASK_SUCCEEDED;
}

/* Função para obter tuplos da tabela.
 * Em caso de erro, devolve NULL.
 */
struct tuple_t **rtable_get(struct rtable_t *rtable, struct tuple_t *template, int keep_tuples, int one_or_all){
    
    int opcode;
    int content_type;
    void * message_content = template;
    
    opcode = assign_opcode(keep_tuples, one_or_all);
    content_type = assign_ctype(opcode);
    
    //cria server_t com base em rtable
    struct server_t *connected_server = server_create_from_rtable(rtable);
    
    //cria mensagem a enviar ao servidor
    struct message_t *client_request = message_create_with(opcode, content_type, message_content);
    
    puts("CLIENT-STUB > RTABLE_GET > Sending message to server...");
    message_print(client_request);
    
    //envia mensagem para o servidor e recebe mensagem do servidor com o resultado da operação
    struct message_t *received_msg = network_send_receive(connected_server, client_request);
    
    /* REVER PORQUE ACHO QUE TEM FALHAS! */
    //verifica se a mensagem recebida foi de sucesso
    if (response_with_success(client_request, received_msg)) {
        //checks what has to do now...
        if ( client_decision_to_take(client_request, received_msg) == CLIENT_RECEIVE_TUPLES ) {
            int number_of_tuples = received_msg->content.result;
            printf("--- has %d tuples to get from the server.\n", number_of_tuples);
            struct tuple_t **received_tuples = (struct tuple_t**) malloc(sizeof(struct tuple_t*)*number_of_tuples);
            
            int i;
            for (i = 0; i < number_of_tuples; i++){
                received_msg = receive_message(connected_server->socketfd);
                received_tuples[i] = tuple_from_message(received_msg);
            }
            return received_tuples;
        }
    }
    return NULL;
}

/* Devolve número de elementos da tabela.
 */
int rtable_size(struct rtable_t *rtable){
    
    int value = 0; //elem to send
    int rtable_size = TASK_FAILED;
    
    //cria server_t com base em rtable
//    struct server_t *connected_server = server_create_from_rtable(rtable);

    //envia mensagem com OC_SIZE, CT_RESULT e element (0)?
    struct message_t *message_to_send = message_create_with(OC_SIZE , CT_RESULT,&value); //SERÁ QUE O 3º CAMPO VAI A NULL?
    
    if(message_to_send == NULL){
        puts("CLIENT-STUB > RTABLE_SIZE > ERROR WHILE CREATING MESSAGE_TO_SEND!.");
        free(message_to_send);
        return TASK_FAILED;
    }

    //recebe mensagem do servidor com o resultado da operação
    struct message_t *received_message = network_send_receive(&rtable->server_to_connect, message_to_send);
    
    //verifica se a mensagem recebida foi de sucesso
    if (response_with_success(message_to_send, received_message) == NO){
        puts("CLIENT-STUB > RTABLE_SIZE > RECEIVED MESSAGE WITH ERROR OPCODE or OPCODE UNEXPECTED.");
        free(message_to_send);
        free(received_message);
        return TASK_FAILED;
    }
    
    //coloca o tamanho da tabela na variavel rtable_size
    rtable_size = received_message->content.result;
//    free (message_to_send);
//    free (received_message);
//    free (connected_server);
    
    return rtable_size;
}

/* Função para reestabelecer uma associação com uma tabela num servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtable_t *rtable_rebind(struct rtable_t *rtable){
    
    int task = TASK_FAILED;
    task = rtable_unbind(rtable); //faz unbind da rtable corrente
    if(task == TASK_FAILED){
        return NULL;
    }

    struct rtable_t *new_rtable;
    new_rtable = rtable_bind(rtable->server_address_and_port); //faz um novo unbind a uma "nova" rtable
    if(new_rtable == NULL){
        return NULL;
    }
    return new_rtable;
}

/*
 * Função que liberta toda a memória alocada a uma estrutura rtable
 */
void rtable_destroy (struct rtable_t *rtable){
    free (rtable->server_address_and_port);
    free (rtable->server_to_connect.ip_address);
    free (rtable);
}

