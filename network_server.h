//
//  network_server.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 26/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#ifndef SD15_Product_network_server_h
#define SD15_Product_network_server_h

#include "message-private.h"



int server_sends_result (int connection_socket_fd, int opcode, int content_type, int value);
 int server_sends_error_msg( int connection_socket_fd);
int server_send_table_size ( int connection_socket_fd, table_t * server );

int server_put (int connection_socket_fd, struct table_t * server, struct message_t * client_message );
int server_send_tuples (int connection_socket_fd, int opcode, struct list_t * matching_nodes);
int server_get_send_tuples ( int connection_socket_fd, table_t * server, struct message_t * cliente_request, int one_or_all);
int server_send_tuple (int connection_socket_fd, int opcode, struct tuple_t * tupleToSend );


/*
 * Recebe um socket fd de comunicação e uma messagem que contem 
 * um pedido por um cliente e processa o pedido e 
 * age em conformidade, enviando uma resposta de volta.
 */
int send_response (struct table_t server, int connection_socket_fd, struct message_t *cliente_request);
/* Esta função é a correspondente à send_receive do network_client.
 * - Recebe um pedido do cliente
 * - Processa a o pedido (deserealiza) e
 * - Envia uma resposta apropriada ao cliente.
 * - Retorna 0 em caso de successo ou -1 em caso de erro.
 */
int network_receive_send(table_t * server_table,  int connection_socket_fd );

#endif
