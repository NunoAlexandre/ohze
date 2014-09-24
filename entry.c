//
//  entry.c
//  SD_2014
//
//  Created by Bruno Mata on 22/09/14.
//
//

#include "tuple.h"
#include "entry.h"
#include <stdlib.h>
#include <string.h> // memcpy


/* Função que cria um novo par chave-valor (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 */
struct entry_t *entry_create(struct tuple_t *tuple){
    struct entry_t *newEntry = (struct entry_t*) malloc(sizeof(struct entry_t));
    newEntry->timestamp = 0;
    newEntry->value = tuple;
    return newEntry;
}

/* Função que destroi um par chave-valor e liberta toda a memoria.
 */
void entry_destroy(struct entry_t *entry){
    free(&(entry->timestamp)); // & = endereço de memória de (entry->timestamp
    tuple_destroy(entry->value);
    free(entry);
}

/* Funcao que duplica um par chave-valor. */
struct entry_t *entry_dup(struct entry_t *entry){
    struct entry_t *duplicatedEntry = (struct entry_t*) malloc(sizeof(struct entry_t));
    memcpy(duplicatedEntry, entry, sizeof (struct entry_t));
    return duplicatedEntry;
    


}
