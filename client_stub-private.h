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

/* Remote table. A definir pelo grupo em client_stub-private.h
 */
struct rtable_t{
    struct server_t server_to_connect;
    char * server_address_and_port;
    int retry_connection;
};

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
struct rtable_t *rtable_rebind(struct rtable_t *rtable);

/*
 * Função que liberta toda a memória alocada a uma estrutura rtable
 */
void rtable_destroy (struct rtable_t *rtable);
#endif
