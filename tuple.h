#ifndef _DATA_H
#define _DATA_H

/* Estrutura que define um tuplo.
 */
struct tuple_t {
    int tuple_dimension; /* Número de elementos no tuplo */
    char **tuple;        /* Conteúdo arbitrário */
};

/* Função que cria um novo tuplo (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 */
struct tuple_t *tuple_create(int tuple_dim);

/* 
 * Função idêntica à anterior, mas com uma assinatura diferente.
 */
struct tuple_t *tuple_create2(int tuple_dim, char **tuple);

/* 
 * Função que destrói um bloco de dados e liberta toda a memoria.
 */
void tuple_destroy(struct tuple_t *tuple);

/* Função que duplica um tuplo. Quando se criam duplicados
 * é necessário efetuar uma CÓPIA dos dados (e não somente alocar a
 * memória necessária).
 */
struct tuple_t *tuple_dup(struct tuple_t *tuple);
#endif
