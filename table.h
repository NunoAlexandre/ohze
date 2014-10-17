#ifndef _TABLE_H
#define _TABLE_H

#include "list-private.h"

struct table_t;   /* A definir pelo grupo em table-private.h */

/* Função para criar/inicializar uma nova tabela hash, com n  
 * linhas(n = módulo da função hash)
 */
struct table_t *table_create(int n);

/* Eliminar/desalocar toda a memória
 */
void table_destroy(struct table_t *table);

/* Função para adicionar um tuplo na tabela.
 * Lembrar que num espaço de tuplos podem existir tuplos iguais. 
 * Devolve 0 (ok) ou -1 (out of memory, outros erros)
 */
int table_put(struct table_t *table, struct tuple_t *tuple);

/* Função para obter um ou todos os tuplos da tabela que
 * estejam de acordo com o template tup_template.
 * Retorna uma lista contendo os tuplos.
 * O argumento keep_tuples indica se o(s) tuplo(s) encontrado(s)
 * deve(m) ser mantido(s) na tabela após a operação.
 * O argumento one_or_all indica se a função deve obter um ou todos
 * os tuplos que estejam de acordo com o template. 
 * Em caso de erro, devolve NULL
 */
struct list_t *table_get(struct table_t *table, struct tuple_t *tup_template, int keep_tuples, int one_or_all);

/* Função para remover um ou todos os tuplos da tabela que estejam
 * de acordo com o template tup_template, e libertar a memória
 * utilizada no armazenamento desse(s) tuplo(s).
 * O argumento one_or_all indica se a função deve remover um ou
 * todos os tuplos que estejam de acordo com o template.
 * Devolve: 0 (ok), -1 (nenhum tuplo encontrado; outros erros)
 */
int table_del(struct table_t *table, struct tuple_t *tup_template, int one_or_all);

/* Devolve o número de elementos na tabela.
 */
int table_size(struct table_t *table);

#endif
