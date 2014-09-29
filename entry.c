//
//  entry.c
//  SD_2014
//
//  Created by Grupo SD015 on 22/09/14.
//
//

#include "tuple.h"
#include "tuple-private.h"
#include "entry.h"
#include <stdlib.h>
#include <string.h> // memcpy


/* Função que cria um novo par chave-valor (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 */
struct entry_t *entry_create(struct tuple_t *tuple){
    struct entry_t *newEntry = (struct entry_t*) malloc(sizeof(struct entry_t));
    if ( newEntry != NULL ) {
        newEntry->timestamp = 0;
        newEntry->value = tuple;
    }
    return newEntry;
}

/* Função que destroi um par chave-valor e liberta toda a memoria.
 */
void entry_destroy(struct entry_t *entry){
    tuple_destroy(entry->value);
}

/* Funcao que duplica um par chave-valor. */
struct entry_t *entry_dup(struct entry_t *entry){
    return  entry_create(tuple_dup(entry->value));
}

/**********  Implementation of entry-private.h   ***********/

/*
 * Returns the key of a given entry.
 */
char * entry_key (struct entry_t * entry) {
    return tuple_key(entry->value);
}

/*
 * Returns the value (tuple) of a given entry.
 */
struct tuple_t * entry_value ( struct entry_t * entry ) {
    return entry->value;
}
