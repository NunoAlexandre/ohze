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

node_t * node_create( struct node_t * prev, struct node_t* next, struct entry_t * entry) ;
node_t * node_createEmpty();
node_t * node_dup(node_t* node);
void node_destroy (struct node_t * node);
int tupleMatchesTemplate ( struct tuple_t * tuple , struct tuple_t * template );
void node_init(node_t* targetNode, node_t * prev, node_t * next, struct entry_t * entry);
int list_removeNode(struct list_t * list, node_t * node, int index);

void node_move(node_t* from, node_t * dest);
char * node_key( node_t * node );
struct entry_t * node_entry(node_t* node);
int entry_keys_compare(struct entry_t * entryA, struct entry_t* entryB);
void node_set_entry(node_t* node, struct entry_t *entry);
void list_size_inc(struct list_t * list) ;
void list_size_dec( struct list_t * list);
node_t * list_tail ( struct list_t* list);
node_t * list_head(struct list_t * list );

void list_print ( struct list_t * list) ;
#endif /* defined(__SD15_Project__list_private__) */

