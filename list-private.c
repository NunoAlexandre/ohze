//
//  list-private.c
//  SD15-Project
//
//  Created by Nuno Alexandre on 24/09/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include "list-private.h"
#include <stdlib.h>

struct node_t * node_create( struct node_t * prev, struct node_t* next, struct entry_t * entry) {
    node_t * newNode = (node_t *) malloc ( sizeof(node_t));
    if ( newNode != NULL ) {
        newNode->prev = prev;
        newNode->next = next;
        newNode->entry = entry_dup(entry);
    }
    
    return newNode;
}

void node_destroy (struct node_t* node ) {
    entry_destroy(node->entry);
    free(node->next);
    free(node->prev);
}