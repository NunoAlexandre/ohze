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
#include "list-private.h"



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
    
    if (keep_tuples == DONT_KEEP_AT_ORIGIN && one_or_all == 1){
        opcode = OC_IN;
    }
    
    else if (keep_tuples == DONT_KEEP_AT_ORIGIN && one_or_all == 0){
        opcode = OC_IN_ALL;
    }
    
    else if (keep_tuples == KEEP_AT_ORIGIN && one_or_all == 1){
        opcode = OC_COPY;
    }
    
    else if (keep_tuples == KEEP_AT_ORIGIN && one_or_all == 0){
        opcode = OC_COPY_ALL;
    }
    
    
    return opcode;
}

/*
 *  Função que dado um endereço cria uma rtable inativa (sem ligação a um servidor)
 */
struct rtable_t * rtable_create_from_address(char* server_address_and_port){
    struct rtable_t *new_rtable = (struct rtable_t*) malloc(sizeof(struct rtable_t));
    if ( new_rtable == NULL)
        return NULL;
    new_rtable->status = RTABLE_UNAVAILABLE;
    new_rtable->server_address_and_port = strdup(server_address_and_port);
    
    return new_rtable;
}


/*
 * Função que dada uma estrutura server_t cria uma rtable
 * retorna NULL em caso de erro
 */
struct rtable_t* rtable_create_from_server ( struct server_t *server_to_connect, char *server_address_and_port){
    
    struct rtable_t *new_rtable = (struct rtable_t*) malloc(sizeof(struct rtable_t));
    if ( new_rtable == NULL)
        return NULL;
    
    char* ip_from_server = strdup(server_to_connect->ip_address);
    int port_number_from_server = server_to_connect->port;
    int socketfd_from_server = server_to_connect->socketfd;
    
    //continuar aqui
    
    new_rtable->server_to_connect.ip_address = ip_from_server;
    free(ip_from_server);
    new_rtable->server_to_connect.port = port_number_from_server;
    new_rtable->server_to_connect.socketfd = socketfd_from_server;
    new_rtable->server_address_and_port = strdup(server_address_and_port);
    new_rtable->status = RTABLE_AVAILABLE;
    
    
    return new_rtable;
}

/*
 * Função que dada uma estrutura rtable cria uma server_t
 * retorna NULL em caso de erro
 */
struct server_t* rtable_get_server ( struct rtable_t *remote_table) {
    return remote_table == NULL ? NULL : &(remote_table->server_to_connect);
}





/* Função para estabelecer uma associação com uma tabela num servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtable_t *rtable_bind(const char *address_port) {
    
    if ( address_port == NULL )
        return NULL;
    
    char* address_port_copy = strdup (address_port);
    struct server_t *server_to_conect = network_connect(address_port_copy);
    
    //verificação da ligação ao servidor
    if (server_to_conect == NULL){
        free(address_port_copy);
        return NULL;
    }
    
    //a criar uma estrutura rtable
    struct rtable_t *remote_table_to_connect = rtable_create_from_server(server_to_conect, address_port_copy);
    
    free(address_port_copy);
    free(server_to_conect);

    return remote_table_to_connect;
}

/* Fecha a ligação com o servidor, liberta toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtable_unbind(struct rtable_t *rtable){
    int task = TASK_FAILED;
    
    //cria server_t com base em rtable
    struct server_t *connected_server = rtable_get_server(rtable);
    
    //verifica se server_t foi bem criado
    if (connected_server == NULL) {
        perror ("CLIENT_STUB > RTABLE_UNBIND > Unable to disconnect from server!");
        return TASK_FAILED;
    }
    
    puts("--- disconnecting from server...");
    
    //faz um network_close ao servidor
    task = network_close(connected_server);
    if(task == TASK_FAILED){
        return TASK_FAILED;
    }
    
    puts("--- disconnected from server...");
    return TASK_SUCCEEDED;
}

/* Função para adicionar um tuplo na tabela.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtable_out(struct rtable_t *rtable, struct tuple_t *tuple){
    
    struct tuple_t *tuple_to_send = tuple;
    
    //cria server_t com base em rtable
    struct server_t *connected_server = rtable_get_server(rtable);
    
    //cria mensagem com OC_OUT, CT_TUPLE e tuple
    struct message_t *message_to_send = message_create_with(OC_OUT, CT_TUPLE, tuple_to_send);
    
    if (message_to_send == NULL){
        puts("CLIENT-STUB > RTABLE_OUT > Failed to create message to send...");
        free(connected_server);
        return TASK_FAILED;
    }
    
    //envia mensagem para o servidor e recebe mensagem do servidor com o resultado da operação
    struct message_t *received_msg = network_send_receive(connected_server, message_to_send);
    
    //faz verificação da mensagem recebida
    if (received_msg == NULL) {
        puts ("CLIENT-STUB > RTABLE_OUT > Failed to send/receive message");
        return TASK_FAILED;
    }
    
    //verifica se a mensagem recebida foi de sucesso
    if (response_with_success(message_to_send, received_msg) == NO){
        puts("CLIENT-STUB > RTABLE_OUT > RECEIVED MESSAGE WITH ERROR OPCODE or OPCODE UNEXPECTED.");
        free(message_to_send);
        free(received_msg);
        return TASK_FAILED;
    }
    
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
    struct server_t *connected_server = rtable_get_server(rtable);
    
    //cria mensagem a enviar ao servidor
    struct message_t *message_to_send = message_create_with(opcode, content_type, message_content);
    
    //envia mensagem para o servidor e recebe mensagem do servidor com o resultado da operação
    struct message_t *received_msg = network_send_receive(connected_server, message_to_send);
    
    //faz verificação da mensagem recebida
    if (received_msg == NULL) {
        puts ("CLIENT-STUB > RTABLE_GET > Failed to send/receive message");
        return NULL;
    }
    
    //onde os tuplos a receber serão guardados
    struct tuple_t **received_tuples = NULL;
    
    //verifica se a mensagem recebida foi de sucesso
    if (response_with_success(message_to_send, received_msg)) {
        //checks what has to do now...
        if ( client_decision_to_take(message_to_send, received_msg) == CLIENT_RECEIVE_TUPLES ) {
            int number_of_tuples = received_msg->content.result;
            printf("--- has %d tuples to get from the server.\n", number_of_tuples);
            received_tuples = (struct tuple_t**) malloc(sizeof(struct tuple_t*)*number_of_tuples);
            
            int i;
            for (i = 0; i < number_of_tuples; i++){
                received_msg = receive_message(connected_server->socketfd);
                received_tuples[i] = tuple_from_message(received_msg);
            }
        }
    }
    else { puts("---- NOT response_with_success !!");  }
    
    //devolve os tuplos recebidos (ou nulo se nao recebeu nenhum tuplo)
    return received_tuples;
}

/* Devolve número de elementos da tabela.
 */
int rtable_size(struct rtable_t *rtable){
    
    int value = 0; //elem to send
    int rtable_size = TASK_FAILED;
    
    //cria server_t com base em rtable
    struct server_t *connected_server = rtable_get_server(rtable);
    
    //envia mensagem com OC_SIZE, CT_RESULT e element
    struct message_t *message_to_send = message_create_with(OC_SIZE , CT_RESULT,&value);
    
    if(message_to_send == NULL){
        free(connected_server);
        return TASK_FAILED;
    }
    
    puts("Sending message to server...");
    
    //envia mensagem para o servidor e recebe mensagem do servidor com o resultado da operação
    struct message_t *received_msg = network_send_receive(connected_server, message_to_send);
    
    
    //faz verificação da mensagem recebida
    if (received_msg == NULL) {
        return TASK_FAILED;
    }
    
    //verifica se a mensagem recebida foi de sucesso
    if (response_with_success(message_to_send, received_msg) == NO){
        free(message_to_send);
        free(received_msg);
        free(connected_server);
        return TASK_FAILED;
    }
    
    //coloca o tamanho da tabela na variavel rtable_size
    rtable_size = received_msg->content.result;
    free (message_to_send);
    free (received_msg);
    
    return rtable_size;
}

/*
 * Função para enviar mensagem OC_REPORT, CT_SFAILURE, para replica a informar que switch não está definido.
 * Recebe mensagem OC_REPORT, CT_Running com informação do endereço do novo switch
 * Devolve 0 (ok) ou -1 (problemas).
 *  (Projeto 5)
 */
char * rtable_report(struct rtable_connection *rtable_connection){
    
    //1. cria mensagem de report com endereço do switch que falhou
    struct message_t * report_to_send = message_create_with(OC_REPORT, CT_SFAILURE, "give me a switch");
    
    if (report_to_send == NULL){
        puts("CLIENT-STUB > RTABLE_REPORT > Failed to create report to send...");
        return NULL;
    }
    
    //    puts("CLIENT-STUB > RTABLE_OUT > Sending message to server...");
    puts("--- sending report to server...");
    
    //2. envia mensagem para o servidor e recebe mensagem do servidor com o resultado da operação
    struct server_t *connected_server = &(rtable_connection->rtable_replica->server_to_connect);
    struct message_t *received_report = network_send_receive(connected_server, report_to_send);
    
    printf(">>> received report with new switch address_port: %s\n", received_report->content.token);
    
    //3. faz verificação da mensagem recebida
    if (received_report == NULL) {
        puts ("CLIENT-STUB > RTABLE_REPORT > Failed to send/receive report");
        return NULL;
    }
    
    //4. verifica se a mensagem recebida foi de sucesso
    if (response_with_success(report_to_send, received_report) == NO){
        puts("CLIENT-STUB > RTABLE_REPORT > RECEIVED MESSAGE WITH ERROR OPCODE or OPCODE UNEXPECTED.");
        free_message2(report_to_send,NO);
        free_message(received_report);
        return NULL;
    }
    
    char * report_content = strdup(received_report->content.token);
    
    free_message2(report_to_send,NO);
    free_message(received_report);

    return report_content;
}

/* Função para reestabelecer uma associação com uma tabela num servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtable_t *rtable_rebind(struct rtable_t *rtable, char* server_address_and_port ){
    
    int task = TASK_SUCCEEDED;
    
    if (rtable != NULL){
        task = rtable_unbind(rtable); //faz unbind da rtable corrente
    }
    
    if(task == TASK_FAILED){
        return NULL;
    }
    
    struct rtable_t *new_rtable;
    new_rtable = rtable_bind(server_address_and_port); //faz um novo unbind a uma "nova" rtable
    if(new_rtable == NULL){
        return NULL;
    }
    //    new_rtable->retry_connection = NO;//ALTEREI AQUI
    return new_rtable;
}

/*
 * Função que liberta toda a memória alocada a uma estrutura rtable
 * (Atualizado para Projeto 5)
 */
void rtable_destroy (struct rtable_t *rtable){
    free (rtable->server_address_and_port);
    free (rtable->server_to_connect.ip_address);
    free (rtable);
}

/*
 *  Inicializa a estrutura rtable_connection
 *  (Projeto 5)
 */
struct rtable_connection* rtable_init (char * config_file){
    
    //1. endereços de todos os servidores
    char ** servers_ip_port = NULL;
    int n_servers = get_system_rtables_info(SYSTEM_CONFIGURATION_FILE,&servers_ip_port);
    if (n_servers == TASK_FAILED) {
        puts("n_servers not enough");
        return NULL;
    }
    
    
    //3.  Vai criar as duas rtables iniciais:
    //3.1 SWITCH
    int switch_position = 0;
    char* switch_address = servers_ip_port[switch_position];
    struct rtable_t *rtable_switch = rtable_bind(switch_address);
    if (rtable_switch == NULL) {
        puts(" if (rtable_switch == NULL)");

        return NULL;
    }
    
    //3.2 REPLICA
    char* replica_address = get_random_replica_address (servers_ip_port, n_servers, -1);
    int replica_position = rtable_connection_find_address(servers_ip_port,n_servers, replica_address);
    
    //2. Cria uma estrutura rtable_connection
    struct rtable_connection* new_rtable_connection = rtable_connection_create(servers_ip_port, n_servers, switch_position, rtable_switch, replica_position, rtable_bind(replica_address) );

    
    free(replica_address);
    
    /* if replica or switch null any error there or before happened and returns null */
    if ( new_rtable_connection->rtable_replica == NULL ||
        new_rtable_connection->rtable_switch == NULL )
    {
        return NULL;
    }
    
    return new_rtable_connection;
}


/* 
 * Função que cria uma nova estrutura rtable_connection (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 * (Projeto 5)
 */
struct rtable_connection * rtable_connection_create(char ** servers_ip_port, int n_servers, int switch_position, struct rtable_t *rtable_switch, int replica_position, struct rtable_t * rtable_replica)
{

    //allocs memory
    struct rtable_connection * new_rtable_connection = (struct rtable_connection*) malloc (sizeof(struct rtable_connection));
    
  
    if ( new_rtable_connection != NULL ) {
        new_rtable_connection->servers_addresses_and_ports = malloc (sizeof(char*) * n_servers);
        if ( new_rtable_connection->servers_addresses_and_ports == NULL  ) {
            rtable_connection_destroy(new_rtable_connection);
            return NULL;
        }
        
        int i; for( i = 0; i < n_servers; i++)
            new_rtable_connection->servers_addresses_and_ports[i] = strdup(servers_ip_port[i]);
    
        //apenas para inicializar tudo
        new_rtable_connection->switch_position = switch_position;
        new_rtable_connection->rtable_switch = rtable_switch;
        new_rtable_connection->replica_position = replica_position;
        new_rtable_connection->rtable_replica = rtable_replica;
    }
    
    return new_rtable_connection;
}


/*
 * Função que atualiza o endereço do switch e rtable_switch
 * Retorna TASK_SUCCEEDED se tudo correu bem
 * (Projeto 5)
 */
int rtable_assign_new_server (struct rtable_t * rtable, char * switch_address_and_port) {
    
    //faz bind a um novo switch
    rtable = rtable_bind(switch_address_and_port);
  
    return rtable != NULL ? TASK_SUCCEEDED : TASK_FAILED;
}


/* AQUIII */
/* atualiza a posição do switch
 int switch_position = rtable_connection_find_address(rtable_connection->servers_addresses_and_ports, rtable_connection->servers_addresses_and_ports, rtable_connection->total switch_address_and_port);
 if (switch_position == TASK_FAILED){
 return TASK_FAILED;
 }*/
//rtable_connection->switch_position = switch_position;


int rtable_connection_server_rebind (struct rtable_connection * rtable_connection, int rebindSwitch ) {
    return rebindSwitch ? rtable_connection_switch_rebind(rtable_connection) : rtable_connection_replica_rebind(rtable_connection);
}

/*
 * Trata de todo o processo de ligação um novo switch:
 * 0. faz unbind do switch atual
 * 1. envia mensagem do tipo REPORT a rtable_replica
 * 2. faz uma ligação ao novo switch
 * (Projeto 5)
 */
int rtable_connection_switch_rebind (struct rtable_connection * rtable_connection){
    int taskSuccess = TASK_FAILED;
    
    //0. faz unbind do switch atual
    taskSuccess = rtable_unbind(rtable_connection->rtable_switch);
    if (taskSuccess == TASK_FAILED) {
        puts("\t--- failed to unbind with current switch");
    }

    //1. envia mensagem do tipo REPORT
    char * new_switch_address = rtable_report(rtable_connection);
    if (new_switch_address == NULL) {
        free(new_switch_address);
        puts("\t--- failed to get new switch");
        return TASK_FAILED;
    }

    //2. faz uma ligação ao novo switch
    taskSuccess = rtable_assign_new_server(rtable_connection->rtable_switch, new_switch_address);
    if (taskSuccess == TASK_FAILED) {
        puts ("\t--- failed to connect to new_switch");
    }
    
    
    /*  updates the switch position */
    rtable_connection->switch_position = rtable_connection_find_address(rtable_connection->servers_addresses_and_ports, rtable_connection->total_servers , new_switch_address);

    
    free(new_switch_address);
    
    return taskSuccess;
}



/*
 * Trata de todo o processo de ligação a uma nova replica
 * 0. faz unbind da replica atual
 * 1. escolhe nova replica aleatoriamente
 * 2. liga-se a nova replica escolhida
 * (Projeto 5)
 */
int rtable_connection_replica_rebind (struct rtable_connection * rtable_connection) {
    int taskSuccess = TASK_FAILED;
    
    //0. faz unbind do switch atual
    rtable_unbind(rtable_connection->rtable_replica);

    //1. envia mensagem do tipo REPORT
    char * new_replica_address =  get_random_replica_address(rtable_connection->servers_addresses_and_ports, rtable_connection->total_servers, rtable_connection->replica_position);
    if (new_replica_address == NULL) {
        free(new_replica_address);
        puts ("FAILED to get new_switch_address");
        return TASK_FAILED;
    }
    
    //2. faz uma ligação ao novo switch
    taskSuccess = rtable_assign_new_server(rtable_connection->rtable_replica, new_replica_address);
    if (taskSuccess == TASK_FAILED) {
        puts ("FAILED to connect to new replica");
    }
    
    /*  updates the switch position */
    rtable_connection->replica_position = rtable_connection_find_address(rtable_connection->servers_addresses_and_ports, rtable_connection->total_servers , new_replica_address);

    
    free(new_replica_address);
    return taskSuccess;
}





/*
 * Encontra em que posição da lista de servers_addresses_and_ports se encontra determinado address_and_port_to_find
 * Retorna a posição ou TASK_FAILED
 * (Projeto 5)
 */
int rtable_connection_find_address (char** addresses_and_ports, int n_servers, char * address_and_port_to_find) {
    int pos = 0;
    for ( pos = 0; pos < n_servers; pos++ ) {
        if (strcmp(addresses_and_ports[pos], address_and_port_to_find) == 0){
            return pos;
        }
        pos++;
    }
    
    //não encontrou na lista
    return TASK_FAILED;
}

/*
 * Inicia o processo de disconectar de uma dada rtable_connection
 * Retorna TASK_SUCCEEDED em caso de sucesso
 * (Projeto 5)
 */
int rtable_disconnect (struct rtable_connection * rtable_connection){
    int task = TASK_FAILED;
    
    //desligar de switch
    struct rtable_t * rtable_switch = rtable_connection_get_switch(rtable_connection);
    
    task = rtable_unbind(rtable_switch);
    free(rtable_switch);
    if (task == TASK_FAILED){
        puts("FAILLED TO UNBIND to SWITCH");
        return task;
    }
    
    //desligar de replica
    struct rtable_t * rtable_replica = rtable_connection_get_replica(rtable_connection);
    
    task = rtable_unbind(rtable_replica);
    free(rtable_replica);
    if (task == TASK_FAILED){
        puts("FAILLED TO UNBIND to REPLICA");
        return task;
    }
    
    return task;
}

/*
 * Liberta toda a memoria alocada a uma estrutura rtable_connection
 * (Projeto 5)
 */
void rtable_connection_destroy (struct rtable_connection * rtable_connection){
    
    if ( rtable_connection != NULL ) {
        rtable_destroy(rtable_connection->rtable_switch);
        rtable_destroy(rtable_connection->rtable_replica);
    
        if (rtable_connection != NULL && rtable_connection->servers_addresses_and_ports != NULL){
            int pos;
            for (pos = 0; pos < rtable_connection->total_servers; pos++){
                if (rtable_connection->servers_addresses_and_ports[pos] != NULL){
                    free (rtable_connection->servers_addresses_and_ports[pos]);
                }
            }
        }
    
        free (rtable_connection->servers_addresses_and_ports);
        free (rtable_connection);
    }
}

/*
 * Retorna o switch de uma dada rtable_connection, NULL em caso de erro
 * (Projeto 5)
 */
struct rtable_t * rtable_connection_get_switch (struct rtable_connection * rtable_connection){
    return rtable_connection->rtable_switch != NULL? rtable_connection->rtable_switch:NULL;
}

/*
 * Retorna o switch de uma dada rtable_connection, NULL em caso de erro
 * (Projeto 5)
 */
struct rtable_t * rtable_connection_get_replica (struct rtable_connection * rtable_connection){
    return rtable_connection->rtable_replica != NULL? rtable_connection->rtable_replica:NULL;
}

/*
 * Random selection of a replica from a server list
 * (Projeto 5)
 */
char* get_random_replica_address (char ** servers_list_address, int n_servers, int current_replica_position) {
    
    int replica_position = current_replica_position;
    //para garantir que o novo switch não é o anterior
    while (replica_position == current_replica_position) {
        replica_position = get_random_number(1, n_servers);
    }
    
    return servers_list_address[replica_position];
}

