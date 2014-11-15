#ifndef _CLIENT_STUB_H
#define _CLIENT_STUB_H

#include "tuple.h"

/* Remote table. A definir pelo grupo em client_stub-private.h 
 */
struct rtable_t; 

/* Função para estabelecer uma associação com uma tabela num servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtable_t *rtable_bind(const char *address_port);

/* Fecha a ligação com o servidor, liberta toda a memória local. 
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtable_unbind(struct rtable_t *rtable);

/* Função para adicionar um tuplo na tabela.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtable_out(struct rtable_t *rtable, struct tuple_t *tuple);

/* Função para obter tuplos da tabela.
 * Em caso de erro, devolve NULL.
 */
struct tuple_t **rtable_get(struct rtable_t *rtable, struct tuple_t *template, int keep_tuples, int one_or_all);

/* Devolve número de elementos da tabela.
 */
int rtable_size(struct rtable_t *rtable);

#endif
