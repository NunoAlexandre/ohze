//
//  client_stub-private.h
//  SD15-Product
//
//  Created by Bruno Mata on 10/11/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#ifndef SD15_Product_client_stub_private_h
#define SD15_Product_client_stub_private_h

#include "network_cliente.h"

#define RTABLE_AVAILABLE YES
#define RTABLE_UNAVAILABLE NO

/*
 * Remote table.
 */
struct rtable_t{
    struct server_t server_to_connect;
    char * server_address_and_port;
    int status;
};

/*
 * Rtable connection
 */
struct rtable_connection {
    char ** servers_addresses_and_ports;
    int total_servers;
    int switch_position;
    struct rtable_t * rtable_switch;
    int replica_position;
    struct rtable_t * rtable_replica;
};

/*
 *  Função que dado um endereço cria uma rtable inativa (sem ligação a um servidor)
 */
struct rtable_t * rtable_create_from_address(char* server_address_and_port);

/*
 * Função que dada uma estrutura server_t cria uma rtable
 * retorna NULL em caso de erro
 */
struct rtable_t* rtable_create_from_server ( struct server_t *server_to_connect, char* server_address_and_port);

/*
 * Função que dada uma estrutura rtable cria uma server_t
 * retorna NULL em caso de erro
 */
struct server_t* server_create_from_rtable ( struct rtable_t *rtable);

/*
 * Função que dada dois argumentos (keep_tuples, on_or_all) define o opcode
 * retorna -1 em caso de erro
 */
int assign_opcode(int keep_tuples, int one_or_all);

/*
 * Assume que a msg_response é uma mensagem de sucesso para com a msg_request.
 */
int client_decision_to_take (struct message_t * msg_request, struct message_t * msg_response);

/*
 * Verifica se o opcode é do tipo Getter
 */
int opcode_is_getter (int opcode);

/* Função para reestabelecer uma associação com uma tabela num servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtable_t *rtable_rebind(struct rtable_t *rtable, char* server_address_and_port);

/*
 * Função que liberta toda a memória alocada a uma estrutura rtable
 * (Atualizado para Projeto 5)
 */
void rtable_destroy (struct rtable_t *rtable);

/*
 * Inicializa a estrutura rtable_connection
 * (Projeto 5)
 */
struct rtable_connection* rtable_init (char * addresses_and_ports);

/*
 * Função que cria uma nova estrutura rtable_connection (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 * (Projeto 5)
 */
struct rtable_connection * rtable_connection_create(char ** servers_ip_port, int n_servers, int switch_position, struct rtable_t *rtable_switch, int replica_position, struct rtable_t * rtable_replica);

/*
 * Função para enviar mensagem OC_REPORT, CT_SFAILURE, para replica a informar que switch não está definido.
 * Recebe mensagem OC_REPORT, CT_Running com informação do endereço do novo switch
 * Devolve 0 (ok) ou -1 (problemas).
 * (Projeto 5)
 */
char * rtable_report(struct rtable_connection *rtable_connection);

/*
 * Função que atualiza o endereço do switch e rtable_switch
 * Retorna TASK_SUCCEEDED em caso de sucesso
 * (Projeto 5)
 */
int rtable_connection_assign_new_switch (struct rtable_connection * rtable_connection, char * switch_address_and_port);

/*
 * Encontra em que posição da lista de servers_addresses_and_ports se encontra determinado address_and_port_to_find
 * Retorna a posição em caso de sucesso ou TASK_FAILED
 * (Projeto 5)
 */
int rtable_connection_find_address (char** addresses_and_ports, int n_servers, char * address_and_port_to_find);

/*
 * Inicia o processo de disconectar de uma dada rtable_connection
 * Retorna TASK_SUCCEEDED em caso de sucesso
 * (Projeto 5)
 */
int rtable_disconnect (struct rtable_connection * rtable_connection);

/*
 * Retorna o switch de uma dada rtable_connection, NULL em caso de erro
 * (Projeto 5)
 */
struct rtable_t * rtable_connection_get_switch (struct rtable_connection * rtable_connection);

/*
 * Retorna o switch de uma dada rtable_connection, NULL em caso de erro
 * (Projeto 5)
 */
struct rtable_t * rtable_connection_get_replica (struct rtable_connection * rtable_connection);

/*
 * Liberta toda a memoria alocada a uma estrutura rtable_connection
 * (Projeto 5)
 */
void rtable_connection_destroy (struct rtable_connection * rtable_connection);

/*
 * Random selection of a replica from a server list
 * (Projeto 5)
 */
char* get_random_replica_address (char ** servers_list_address, int n_servers, int current_replica_position);

/*
 * Procedimentos para estabelecer uma nova ligação a um novo rtable_switch
 * (Projeto 5)
 */
int rtable_bind_new_switch (struct rtable_connection * rtable_connection);


/*
 * Rebinds the rtable_connection switch or replica to a new one, depending on rebindSwitch or not.
 */
int rtable_connection_server_rebind (struct rtable_connection * rtable_connection, int rebindSwitch );

/*
 * Trata de todo o processo de ligação um novo switch:
 * 0. faz unbind do switch atual
 * 1. envia mensagem do tipo REPORT a rtable_replica
 * 2. faz uma ligação ao novo switch
 * (Projeto 5)
 */
int rtable_connection_switch_rebind (struct rtable_connection * rtable_connection);

/*
 * Trata de todo o processo de ligação a uma nova replica
 * 0. faz unbind da replica atual
 * 1. escolhe nova replica aleatoriamente
 * 2. liga-se a nova replica escolhida
 * (Projeto 5)
 */
int rtable_connection_replica_rebind (struct rtable_connection * rtable_connection);

#endif