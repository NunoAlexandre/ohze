//
//  list-private.h
//  SD15-Project
//
//  Created by Nuno Alexandre on 24/09/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#ifndef __SD15_Project__list_private__
#define __SD15_Project__list_private__

#include <stdio.h>
#include "entry.h"
#include "entry-private.h"

struct list_t {
    int  size;
    struct node_t *head;
    struct node_t *tail;
};

typedef struct node_t {
    struct node_t *prev;
    struct node_t *next;
    struct entry_t *entry;
} node_t;

struct node_t * node_create( struct node_t * prev, struct node_t* next, struct entry_t * entry) ;

void node_destroy (struct node_t * node);

#endif /* defined(__SD15_Project__list_private__) */

