#ifndef _LIST_H
#define _LIST_H

#include "entry.h"

struct list_t; /*A definir pelo grupo em list-private.h*/

/* Cria uma nova lista. Em caso de erro, retorna NULL.
 */
struct list_t *list_create();

/* Elimina uma lista, libertando *toda* a memoria utilizada pela
 * lista.
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_destroy(struct list_t *list);

/* Adiciona uma entry na lista. Como a lista deve ser ordenada,
 * a nova entry deve ser colocada no local correto.
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_add(struct list_t *list, struct entry_t *entry);

/* Elimina da lista um elemento (tuplo) de acordo com o padrão
 * tup_template.
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_remove(struct list_t *list, struct tuple_t *tup_template);

/* Obtem um elemento (tuplo) da lista de acordo com o padrão
 * tup_template.
 * Retorna a referencia do elemento na lista (ou seja, uma alteração
 * implica alterar o elemento na lista).
 */
struct entry_t *list_get(struct list_t *list, struct tuple_t *tup_template);

/* Retorna o tamanho (numero de elementos) da lista
 * Retorna -1 em caso de erro.
 */
int list_size(struct list_t *list);

#endif
