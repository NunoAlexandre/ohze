#ifndef _ENTRY_H
#define _ENTRY_H

#include "tuple.h"

/* Esta estrutura define o par chave-valor para a tabela */

struct entry_t {
    /* char *key;  string, (char* terminado por '\0'). Nestes
     projetos não será usado  pois coincide
     com primeiro elemento do tuplo */
    long long timestamp; /* Por agora vai ter valor igual a 0.
                          Será utilizado no projeto 5. */
    struct tuple_t *value; /* Bloco de dados. Um tuplo neste caso. */
};

/* Função que cria um novo par chave-valor (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 */
struct entry_t *entry_create(struct tuple_t *tuple);

/* Função que destroi um par chave-valor e liberta toda a memoria.
 */
void entry_destroy(struct entry_t *entry);

/* Funcao que duplica um par chave-valor. */
struct entry_t *entry_dup(struct entry_t *entry);

#endif
