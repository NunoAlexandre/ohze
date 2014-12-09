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

int current_replica_position;
int current_switch_position;


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
    
    else{
        opcode = TASK_FAILED;
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
    
    //    puts ("CLIENTE_STUB > RTABLE_CREATE_FROM_SERVER > Creating new rtable...");
    
    char* ip_from_server = strdup(server_to_connect->ip_address);
    int port_number_from_server = server_to_connect->port;
    int socketfd_from_server = server_to_connect->socketfd;
    
    //continuar aqui
    
    new_rtable->server_to_connect.ip_address = ip_from_server;
    free(ip_from_server);
    new_rtable->server_to_connect.port = port_number_from_server;
    new_rtable->server_to_connect.socketfd = socketfd_from_server;
    new_rtable->server_address_and_port = strdup(server_address_and_port);
    //    new_rtable->retry_connection = YES; //ALTEREI AQUI
    
    //    puts ("CLIENTE_STUB > RTABLE_CREATE_FROM_SERVER > New rtable created...");
    
    return new_rtable;
}

/*
 * Função que dada uma estrutura rtable cria uma server_t
 * retorna NULL em caso de erro
 */
struct server_t* server_create_from_rtable ( struct rtable_t *remote_table){
    
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
    free(ip_from_rtable);
    new_server->port = port_number_from_rtable;
    new_server->socketfd = socketfd_from_rtable;
    
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
    
    
    //verificação da ligação ao servidor
    if (server_to_conect == NULL){
        free(address_port_copy);
        free(server_to_conect);
        return NULL; // neste momento estará a NULL
    }
    
    //a criar uma estrutura rtable
    struct rtable_t *remote_table_to_connect = rtable_create_from_server(server_to_conect, address_port_copy);
    
    free(address_port_copy);
    free(server_to_conect);
    
    puts("CONNECTED TO SERVER");
    remote_table_to_connect->status = RTABLE_AVAILABLE;
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
        return TASK_FAILED;
    }
    
    puts("Disconnecting from server...");
    
    //faz um network_close ao servidor
    task = network_close(connected_server);
    if(task == TASK_FAILED){
        return TASK_FAILED;
    }
    
    free(connected_server);
    puts("Disconnected from server...");
    return TASK_SUCCEEDED;
}

/* Função para adicionar um tuplo na tabela.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtable_out(struct rtable_t *rtable, struct tuple_t *tuple){
    
    struct tuple_t *tuple_to_send = tuple;
    
    //cria server_t com base em rtable
    struct server_t *connected_server = server_create_from_rtable(rtable);
    
    //cria mensagem com OC_OUT, CT_TUPLE e tuple
    struct message_t *message_to_send = message_create_with(OC_OUT, CT_TUPLE, tuple_to_send);
    
    if (message_to_send == NULL){
        puts("CLIENT-STUB > RTABLE_OUT > Failed to create message to send...");
        free(connected_server);
        return TASK_FAILED;
    }
    
    //    puts("CLIENT-STUB > RTABLE_OUT > Sending message to server...");
    puts("Sending message to server...");
    
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
    struct server_t *connected_server = server_create_from_rtable(rtable);
    
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
    struct server_t *connected_server = server_create_from_rtable(rtable);
    
    //envia mensagem com OC_SIZE, CT_RESULT e element
    struct message_t *message_to_send = message_create_with(OC_SIZE , CT_RESULT,&value);
    
    if(message_to_send == NULL){
        free(message_to_send);
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
    free (connected_server);
    
    return rtable_size;
}

/*
 * Função para enviar mensagem OC_REPORT, CT_SFAILURE, para replica a informar que switch não está definido.
 * Recebe mensagem OC_REPORT, CT_Running com informação do endereço do novo switch
 * Devolve 0 (ok) ou -1 (problemas).
 *  (Projeto 5)
 */
char * rtable_report(struct rtable_connection *system_init){
    
    int replica_address = system_init->replica_position; //replica address position
    struct server_t *connected_replica = server_create_from_rtable (system_init->rtable_replica);
    
    //1. cria mensagem de report com endereço do switch que falhou
    struct message_t * report_to_send = message_create_with(OC_REPORT, CT_SFAILURE, system_init->servers_addresses_and_ports[replica_address]);
    
    if (report_to_send == NULL){
        puts("CLIENT-STUB > RTABLE_REPORT > Failed to create report to send...");
        free(connected_replica);
        return NULL;
    }
    
    //    puts("CLIENT-STUB > RTABLE_OUT > Sending message to server...");
    puts("Sending message to server...");
    
    //2. envia mensagem para o servidor e recebe mensagem do servidor com o resultado da operação
    struct message_t *received_report = network_send_receive(connected_replica, report_to_send);
    
    //3. faz verificação da mensagem recebida
    if (received_report == NULL) {
        puts ("CLIENT-STUB > RTABLE_REPORT > Failed to send/receive report");
        return NULL;
    }
    
    //4. verifica se a mensagem recebida foi de sucesso
    if (response_with_success(report_to_send, received_report) == NO){
        puts("CLIENT-STUB > RTABLE_REPORT > RECEIVED MESSAGE WITH ERROR OPCODE or OPCODE UNEXPECTED.");
        free(report_to_send);
        free(received_report);
        return NULL;
    }
    
    //devolve o novo endereço
    return received_report->content.token;
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
struct rtable_connection* rtable_init (char * addresses_and_ports){
    
    //1. endereços de todos os servidores
    char ** servers_addresses_and_ports = NULL;
    int total_servers = get_system_rtables_info(SYSTEM_CONFIGURATION_FILE,&servers_addresses_and_ports);
    if (total_servers == TASK_FAILED) {
        return NULL;
    }
    
    //2. Cria uma estrutura rtable_connection
    struct rtable_connection* new_rtable_connection = rtable_connection_create(total_servers);
    
    //3.  Vai criar as duas rtables iniciais:
    //3.1 SWITCH
    int switch_position = 0;
    char* switch_address = servers_addresses_and_ports[switch_position];
    struct rtable_t *rtable_switch = rtable_bind(switch_address);
    if (rtable_switch == NULL){
        free (new_rtable_connection);
        return NULL;
    }
    
    //3.2 REPLICA
    char* replica_address = get_server_replica_address (servers_addresses_and_ports, total_servers);
    struct rtable_t *rtable_replica = rtable_bind(replica_address);
    if (rtable_replica == NULL){
        free (new_rtable_connection);
        free (rtable_switch);
        return NULL;
    }
    
    //4. Preenche uma estrutura rtable_connection
    new_rtable_connection->servers_addresses_and_ports = servers_addresses_and_ports;
    new_rtable_connection->total_servers = total_servers;
    new_rtable_connection->switch_position = switch_position;
    new_rtable_connection->rtable_switch = rtable_switch;
    new_rtable_connection->replica_position = rtable_connection_find_address(new_rtable_connection, replica_address);
    new_rtable_connection->rtable_replica = rtable_replica;
    
    //5. Atualiza as variaveis referentes às posições da replica e switch
    current_switch_position = switch_position;
    current_replica_position = new_rtable_connection->replica_position;
    
    return new_rtable_connection;
}


/* 
 * Função que cria uma nova estrutura rtable_connection (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 * (Projeto 5)
 */
struct rtable_connection * rtable_connection_create(int n_servers) {
    //checks if required n_servers is valid
    if ( n_servers <= 0)
        return NULL;
    
    //allocs memory
    struct rtable_connection * new_rtable_connection = (struct rtable_connection*) malloc (sizeof(struct rtable_connection));
    
    if ( new_rtable_connection != NULL ) {
        new_rtable_connection->servers_addresses_and_ports = (char **) malloc(n_servers * sizeof(char*));
        if (new_rtable_connection->servers_addresses_and_ports == NULL){
            free(new_rtable_connection);
            return NULL;
        }
        
        //apenas para inicializar tudo
        new_rtable_connection->switch_position = 0;
        new_rtable_connection->rtable_switch = NULL;
        new_rtable_connection->replica_position = 0;
        new_rtable_connection->rtable_switch = NULL;
    }
    return new_rtable_connection;
}


/*
 * Função que atualiza o endereço do switch e rtable_switch
 * Retorna TASK_SUCCEEDED se tudo correu bem
 * (Projeto 5)
 */
int rtable_connection_assign_new_switch (struct rtable_connection * system_init, char * switch_address_and_port){
    
    //faz bind a um novo switch
    struct rtable_t * new_rtable_switch = rtable_bind(switch_address_and_port);
    if (new_rtable_switch == NULL){
        return TASK_FAILED;
    }
    system_init->rtable_switch = new_rtable_switch;
    
    //atualiza a posição do switch
    int switch_position = rtable_connection_find_address(system_init, switch_address_and_port);
    if (switch_position == TASK_FAILED){
        return TASK_FAILED;
    }
    system_init->switch_position = switch_position;
    
    return TASK_SUCCEEDED;
}

/*
 * Trata de todo o processo de ligação um novo switch:
 * 0. faz unbind do switch atual
 * 1. envia mensagem do tipo REPORT a rtable_replica
 * 2. faz uma ligação ao novo switch
 * (Projeto 5)
 */
int rtable_connection_switch_rebind (struct rtable_connection * system_init){
    int taskSuccess = TASK_FAILED;
    
    //0. faz unbind do switch atual
    taskSuccess = rtable_unbind(system_init->rtable_switch);
    if (taskSuccess == TASK_FAILED) {
        puts("FAILED TO UNBIND WITH CURRENT SWITCH!");
    }

    //1. envia mensagem do tipo REPORT
    char * new_switch_address = strdup (rtable_report(system_init));
    if (new_switch_address == NULL) {
        free(new_switch_address);
        puts ("FAILED to get new_switch_address");
        return TASK_FAILED;
    }

    //2. faz uma ligação ao novo switch
    taskSuccess = rtable_connection_assign_new_switch(system_init, new_switch_address);
    if (taskSuccess == TASK_FAILED) {
        puts ("FAILED to connect to new_switch");
        free(new_switch_address);
        return TASK_FAILED;
    }
    
    return taskSuccess;
}

/*
 * Encontra em que posição da lista de servers_addresses_and_ports se encontra determinado address_and_port_to_find
 * Retorna a posição ou TASK_FAILED
 * (Projeto 5)
 */
int rtable_connection_find_address (struct rtable_connection * system_init, char * address_and_port_to_find){
    
    char ** address_and_port_list = system_init->servers_addresses_and_ports;
    int total_servers = system_init->total_servers;
    
    int pos = 0;
    
    for ( pos = 0; pos < total_servers; pos++ ) {
        printf("%s\n",address_and_port_list[pos]);
        if (strcmp(address_and_port_list[pos], address_and_port_to_find) == 0){
            printf("ENCONTREI!\n");
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
int rtable_disconnect (struct rtable_connection * system_init){
    int task = TASK_FAILED;
    
    //desligar de switch
    struct rtable_t * rtable_switch = rtable_connection_get_switch(system_init);
    
    task = rtable_unbind(rtable_switch);
    free(rtable_switch);
    if (task == TASK_FAILED){
        puts("FAILLED TO UNBIND to SWITCH");
        return task;
    }
    
    //desligar de replica
    struct rtable_t * rtable_replica = rtable_connection_get_replica(system_init);
    
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
void rtable_connection_destroy (struct rtable_connection * system_init){
    
    rtable_destroy(system_init->rtable_switch);
    rtable_destroy(system_init->rtable_replica);
    
    if (system_init != NULL && system_init->servers_addresses_and_ports != NULL){
        int pos;
        for (pos = 0; pos < system_init->total_servers; pos++){
            if (system_init->servers_addresses_and_ports[pos] != NULL){
                free (system_init->servers_addresses_and_ports[pos]);
            }
        }
    }
    
    free (system_init->servers_addresses_and_ports);
    free (system_init);
}

/*
 * Retorna o switch de uma dada rtable_connection, NULL em caso de erro
 * (Projeto 5)
 */
struct rtable_t * rtable_connection_get_switch (struct rtable_connection * system_init){
    return system_init->rtable_switch != NULL? system_init->rtable_switch:NULL;
}

/*
 * Retorna o switch de uma dada rtable_connection, NULL em caso de erro
 * (Projeto 5)
 */
struct rtable_t * rtable_connection_get_replica (struct rtable_connection * system_init){
    return system_init->rtable_replica != NULL? system_init->rtable_replica:NULL;
}

/*
 * Random selection of a replica from a server list
 * (Projeto 5)
 */
char* get_server_replica_address (char ** servers_list_address, int n_servers){
    int replica_position = current_replica_position;
    
    //para garantir que o novo switch não é o anterior
    while (replica_position == current_replica_position){
        replica_position = get_random_number(1, n_servers-1);
    }

    //actualiza a variavel current_replica_position com a nova posição da replica
    current_replica_position = replica_position;
    return servers_list_address[replica_position];
}

/*
 * Procedimentos para estabelecer uma nova ligação a um novo rtable_switch
 * (Projeto 5)
 */
int rtable_bind_new_switch (struct rtable_connection * system_init){
    int task = TASK_FAILED;
    
    //1. Faz unbind com o switch corrente
    task = rtable_unbind(system_init->rtable_switch);
    if (task == TASK_FAILED) {
        puts ("RTABLE_BIND_NEW_SWITCH -> Failed to unbind with current switch!");
    }
    
    //2. Envia mensagem do tipo REPORT
    char * new_switch_address = strdup (rtable_report(system_init));
        if (new_switch_address == NULL){
            free(new_switch_address);
            return TASK_FAILED;
        }
    
    //3. Atualiza rtable_connection com o novo switch_address
    system_init->rtable_switch = rtable_bind(new_switch_address);
    if (system_init->rtable_switch == NULL) {
        puts ("RTABLE_BIND_NEW_SWITCH -> Failed to bind with new switch!");
        free(new_switch_address);
        return TASK_FAILED;
    }
    
    int switch_position = rtable_connection_find_address(system_init, new_switch_address);
    system_init->switch_position = switch_position; //actualiza posição do switch
    current_switch_position = switch_position; //actualiza a variavel current_switch_position com a nova posição do switch
    
    free(new_switch_address);
    return TASK_SUCCEEDED;
}

