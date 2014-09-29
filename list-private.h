//
//  list-private.h
//  SD15-Project
//
//  Created by Nuno Alexandre on 24/09/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#ifndef __SD15_Project__list_private__
#define __SD15_Project__list_private__

#include <stdio.h>
#include "entry.h"
#include "entry-private.h"

/*
 * Structure that defines a list
 */
struct list_t {
    int  size;
    struct node_t *head;
    struct node_t *tail;
};
/*
 * Structure that defines a doubly linked-list node.
 */
typedef struct node_t {
    struct node_t *prev;
    struct node_t *next;
    struct entry_t *entry;
} node_t;

/*
 * Creates a node having the prev and next node and its entry.
 */
node_t * node_create( struct node_t * prev, struct node_t* next, struct entry_t * entry) ;
/*
 * Creates an empty node - just allocates memory for the structure.
 */
node_t * node_create_empty();
/*
 * Duplicates a node.
 */
node_t * node_dup(node_t* node);
/*
 * Destroyes a node.
 */
void node_destroy (struct node_t * node);
/*
 * Method that checks if a certain tuple matches a template.
 * If the tuple and the template have different sizes they dont match.
 * If a field of the template is not null and is not equal to element
 * at same position of the tuple, they dont match.
 * Returns 1 (true) if they match, 0 otherwise.
 */
int tuple_matches_template ( struct tuple_t * tuple , struct tuple_t * template );



/*
 * Method thats inserts a node in the list, beforeOrAfter aNode.
 * If beforeOrAfter is 0, inserts before aNode,  if 1 adds it after it.
 * It can get aNode null if list_size(list) == 0.
 * Returns 0 in success case, -1 in error case.
 */
int list_insert_node(struct list_t* list,  node_t * newNode, node_t* aNode, int beforeOrAfter);

/*
 * Method that gets a node from the given list that matches the tup_template.
 * It returns the first matching node (head to tail) or NULL if none match.
 */
node_t *list_matching_node(struct list_t *list, struct tuple_t *tup_template);

/*
 * Checks if list is empty. 
 * Returns 1 if its empty, 0 otherwise.
 */
int list_isEmpty(struct list_t* list);

/* Returns the size of the list - the number of elements.
 * Retorna -1 em caso de erro.
 */
int list_size(struct list_t *list);

/*
 * Method that returns the key of the node. 
 * Delegates the request to the type of entry it uses.
 */
char * node_key( node_t * node );
/*
 * Method that returns the entry of a given node.
 */
struct entry_t * node_entry(node_t* node);
/*
 * Method that compares two entry keys with strcmp 
 * on the same order than the parameters are received.
 */
int entry_keys_compare(struct entry_t * entryA, struct entry_t* entryB);
/*
 * Method that increments +1 to the size of the the given list.
 */
void list_size_inc(struct list_t * list);
/*
 * Method that decrements -1 to the size of the given list.
 */
void list_size_dec( struct list_t * list);
/*
 * Method that returns the tail of the given list.
 */
node_t * list_tail ( struct list_t* list);
/*
 * Method that returns the head of the given list.
 */
node_t * list_head(struct list_t * list );

/*
 * Method to print the given list.
 */
void list_print ( struct list_t * list) ;

#endif /* defined(__SD15_Project__list_private__) */

