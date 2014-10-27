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

/*
 * Recebe um socket fd de comunicação e uma messagem que contem 
 * um pedido por um cliente e processa o pedido e 
 * age em conformidade, enviando uma resposta de volta.
 */
int send_response (int connection_socket_fd, struct message_t *cliente_request);

/* Esta função é a correspondente à send_receive do network_client.
 * - Recebe um pedido do cliente
 * - Processa a o pedido (deserealiza) e
 * - Envia uma resposta apropriada ao cliente.
 * - Retorna 0 em caso de successo ou -1 em caso de erro.
 */
int network_receive_send(int connection_socket_fd);

/* A funcao network_close() deve fechar a ligação estabelecida por
 * network_connect(). Se network_connect() alocou memoria, a função
 * deve libertar essa memoria.
 */
int network_close(struct server_t *server);


#endif
