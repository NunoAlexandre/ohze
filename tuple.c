//
//  tuple.c
//  sd15project
//
//  Created by Nuno Alexandre on 22/09/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include "tuple.h"

struct tuple_t *tuple_create(int tuple_dim) {
    struct tuple_t * newTuple = (struct tuple_t*) malloc (sizeof(struct tuple_t));
    
    if ( newTuple != NULL ) {
        newTuple->tuple_dimension = tuple_dim;
        newTuple->tuple = (char**) malloc (sizeof(char*)*tuple_dim);
    }
    return newTuple;
}

struct tuple_t *tuple_create2(int tuple_dim, char **tuple) {
    struct tuple_t * newTuple = (struct tuple_t*) malloc (sizeof(struct tuple_t));
    
    if ( newTuple != NULL ) {
        newTuple->tuple_dimension = tuple_dim;
        newTuple->tuple = (char**) malloc (sizeof(char*)*tuple_dim +1);
        for ( int i=0; i<newTuple->tuple_dimension; i++) {
            newTuple->tuple[i] = strdup(tuple[i]);
        }
    }
    return newTuple;
}

void tuple_destroy(struct tuple_t *tuple) {
    free( &(tuple->tuple_dimension));
    int i;
    for ( i = 0; i < tuple->tuple_dimension; i++ ) {
        free(tuple->tuple[i]);
    }
    free(tuple->tuple);
    free(tuple);
}

struct tuple_t *tuple_dup (struct tuple_t *tuple) {
    return tuple_create2(tuple->tuple_dimension, tuple->tuple);
}
