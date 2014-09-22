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
    
    newTuple->tuple_dimension = tuple_dim;
    //acho que nao faz sentido reservar memoria...
    newTuple->tuple = (char**) malloc (sizeof(char)*tuple_dim);
    
    return newTuple;
}

struct tuple_t *tuple_create2(int tuple_dim, char **tuple) {
    struct tuple_t * newTuple = (struct tuple_t*) malloc (sizeof(struct tuple_t));
    newTuple->tuple_dimension = tuple_dim;
    newTuple->tuple = (char**) malloc (sizeof(char*)*tuple_dim);
    memcpy( newTuple->tuple, tuple, sizeof(char) * sizeof(tuple));
    
    return newTuple;
}

void tuple_destroy(struct tuple_t *tuple) {
    free( &(tuple->tuple_dimension));
    free(tuple->tuple);
    free(tuple);
}

struct tuple_t *tuple_dup (struct tuple_t *tuple) {
    struct tuple_t * duplicatedTuple = (struct tuple_t*) malloc (sizeof(struct tuple_t));
    memcpy(duplicatedTuple, tuple, sizeof(struct tuple_t));
    return duplicatedTuple;
}
