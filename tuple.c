//
//  tuple.c
//  sd15project
//
//  Created by Nuno Alexandre on 22/09/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tuple.h"

/* Função que cria um novo tuplo (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 */
struct tuple_t *tuple_create(int tuple_dim) {
    struct tuple_t * newTuple = (struct tuple_t*) malloc (sizeof(struct tuple_t));
    
    if ( newTuple != NULL ) {
        newTuple->tuple_dimension = tuple_dim;
        newTuple->tuple = (char**) malloc (sizeof(char*)*tuple_dim);
    }
    return newTuple;
}
/*
 * Função idêntica à anterior, mas com uma assinatura diferente.
 */
struct tuple_t *tuple_create2(int tuple_dim, char **tuple) {
    struct tuple_t * newTuple = tuple_create(tuple_dim);
    //if it was allocated we just need to copy the tuple values
    if ( newTuple != NULL ) {
        for ( int i=0; i<newTuple->tuple_dimension; i++) {
            newTuple->tuple[i] = strdup(tuple[i]);
        }
    }
    return newTuple;
}

/*
 * Função que destrói um bloco de dados e liberta toda a memoria.
 */
void tuple_destroy(struct tuple_t *tuple) {
    for ( int i = 0; i < tuple->tuple_dimension; i++ ) {
        free(tuple->tuple[i]);
    }
    tuple = NULL;
}

/* 
 * Função que duplica um tuplo. Quando se criam duplicados
 * é necessário efetuar uma CÓPIA dos dados (e não somente alocar a
 * memória necessária).
 */
struct tuple_t *tuple_dup (struct tuple_t *tuple) {
    return tuple_create2(tuple->tuple_dimension, tuple->tuple);
}


/*********   Implementation of tuple-private.h    **********/

/*
 * Method that returns the key of a given tuple.
 */
char * tuple_key (struct tuple_t * tuple ) {
    return tuple->tuple[0];
}

/*
 * Method that returns the size of a given tuple.
 */
int tuple_size( struct tuple_t * tuple ) {
    return tuple->tuple_dimension;
}

/*
 * Method that returns the iElement of a given tuple.
 */
char * tuple_element ( struct tuple_t * tuple, int iElement ) {
    return tuple->tuple[iElement];
}