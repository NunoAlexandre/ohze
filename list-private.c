//
//  list-private.c
//  SD15-Project
//
//  Created by Nuno Alexandre on 24/09/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include "list-private.h"
#include "tuple-private.h"
#include <string.h>
#include <stdlib.h>

node_t * node_create( struct node_t * prev, struct node_t* next, struct entry_t * entry) {
    node_t * newNode = (node_t *) malloc ( sizeof(node_t));
    if ( newNode != NULL ) {
        newNode->prev =  prev;
        newNode->next = next;
        newNode->entry = entry_dup(entry);
    }
    
    return newNode;
}

node_t * node_dup(node_t* node) {
    return node_create(node->prev, node->next, entry_dup(node->entry));
}

node_t * node_createEmpty() {
    node_t * newNode = (node_t *) malloc ( sizeof(node_t));
    return newNode;
}


void node_destroy (struct node_t* node ) {
    entry_destroy(node->entry);
    node->prev = NULL;
    node->next = NULL;
}

/*
 * Checks if tuple matches a template.
 * Returns 1 (true) if matches, 0 otherwise.
 */
int tuple_matches_template ( struct tuple_t * tuple , struct tuple_t * template ) {
    if ( tuple_size(tuple) != tuple_size(template) )
        return 0;
    
    //Flag to say if the tuple matchs the template.
    // if doesnt, matches turns 0 (false), otherwise it keeps 1.
    int matches = 1;
    // saves the number of checked elements of the tuple
    int iElement = 0;
    
    while ( matches && iElement < tuple_size(tuple) ) {
        //tuple iElement
        char * tupleElement = tuple_element(tuple, iElement);
        //template iElement
        char * templateElement = tuple_element(template, iElement);
        
        //if templateElement is not null but not equal to the tupleElement, doesnt match.
        if ( strcmp(templateElement, "NULL") != 0  && strcmp(tupleElement, templateElement) != 0 )
            matches = 0;
        
        iElement++;
    }
    //if the match checking never failed it matches (0)
    return matches;
}


void list_print ( struct list_t * list) {
    if ( list->size == 0 ) {
        printf("list_print : list is empty \n");
    }
    else {
        node_t * nodeToPrint = list->head;
        //char * nodeKey = node_key(nodeToPrint);

        if ( list->size == 1 ) {
            printf("list_print : list has only one element with key: %s \n", entry_key(nodeToPrint->entry));
        }
        else {
             int nodesToPrint = list->size;
           // printf("************* Print list with SIZE: %d\n", nodesToPrint);
            while ( nodesToPrint--  > 0 ) {
                printf("node with key: %s and pointer %p\n", entry_key(nodeToPrint->entry), nodeToPrint);
                nodeToPrint = nodeToPrint->next;
            }
            printf("1. head node with key: %s and pointer %p\n", entry_key(list->head->entry), entry_key(list->head->entry));
            //printf("1. head next node with key: %s and pointer %p\n", entry_key(list->head->next->entry),entry_key(list->head->next->entry) );
            printf("1. tail node with key: %s and pointer %p\n", entry_key(list->tail->entry),entry_key(list->tail->entry));
            //printf("2. before tail node with key: %s and pointer %p\n", entry_key(list->tail->prev->entry),entry_key(list->tail->prev->entry));
            //printf("2. prev prev tail node with key: %s and pointer %p\n", entry_key(list->tail->prev->prev->entry),entry_key(list->tail->prev->prev->entry));

        }
    }
    //printf("1. print head->next->next->next %s\n", entry_key(list->head->next->next->next->entry));

}

void node_init(node_t* targetNode, node_t * prev, node_t * next, struct entry_t * entry) {
    if ( targetNode != NULL ) {
        targetNode->next = targetNode;
        targetNode->prev = targetNode;
        targetNode->entry = entry_dup(entry);
        printf("enter node_init with key %s \n", entry_key(targetNode->entry));

    }
}

char * node_key( node_t * node ) {
    return entry_key(node->entry);
}

void node_move(node_t* from, node_t * dest) {
    //from = dest;
}


void list_size_inc(struct list_t * list) {
    list->size++;
}

void list_size_dec( struct list_t * list) {
    list->size--;
}
node_t * list_tail ( struct list_t* list) {
    return list->tail;
}
node_t * list_head(struct list_t * list ) {
    return list->head;
}
struct entry_t * node_entry(node_t* node) {
    return node->entry;
}

void node_set_entry(node_t* node, struct entry_t *entry) {
    node->entry = entry_dup(entry);
}
