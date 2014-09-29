//
//  main.c
//  SD
//
//  Created by Grupo SD015 on 22/09/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tuple.h"
#include "list.h"
#include "list-private.h"

int main(int argc, const char * argv[]) {

    //first entry
    char ** ElementsForTupleA = (char**) malloc(sizeof(char*)*3);
    ElementsForTupleA[0] = strdup("Nuno");
    ElementsForTupleA[1] = strdup("Miguel");
    ElementsForTupleA[2] = strdup("Alexandre");
    struct tuple_t * tupleA = tuple_create2(3, ElementsForTupleA);
    struct entry_t * entryA = entry_create(tupleA);
    //second entry
    char ** ElementsForTupleB = (char**) malloc(sizeof(char*)*3);
    ElementsForTupleB[0] = strdup("Lieske");
    ElementsForTupleB[1] = strdup("De");
    ElementsForTupleB[2] = strdup("Wilde");
    struct tuple_t * tupleB = tuple_create2(3, ElementsForTupleB);
    struct entry_t * entryB = entry_create(tupleB);

    
    struct list_t * list = list_create();
    
   
    list_add(list, entryA);
    list_print(list);
    
    struct entry_t * entryA2 = entry_create(tuple_dup(tupleA));
    entryA2->value->tuple[0] = strdup("Nuno2");

    list_add(list, entry_dup(entryA2));
    list_print(list);
    
    list_add(list, entryB);
    list_print(list);
    
  //  printf(" HEEEEE : entryA has key %s\n", entry_key(entryA));
    
    //list_add(list, entry_dup(entryA));
  //  list_print(list);

   // list_add(list, entryB);
    //list_print(list);
    
    struct entry_t * entryC = entry_dup(entryB);
    entryC->value->tuple[0] = strdup("Zé!");

    list_add(list, entryC);
    list_print(list);
    
   /* list_add(list, entryB);
    list_print(list);*/
    
    struct entry_t * entryD = entry_dup(entryC);
    entryD->value->tuple[0] = strdup("Ana");
    
    list_add(list, entryD);
    list_print(list);
    
    struct entry_t * entryE = entry_dup(entryD);
    entryE->value->tuple[0] = strdup("Za");
    
    list_add(list, entryE);
    list_add(list, entryE);
    list_add(list, entryC);
    
    struct entry_t * entryF = entry_dup(entryD);
    entryF->value->tuple[0] = strdup("Aane");
    
    list_add(list, entryF);

    list_print(list);
    
    
    printf(" ##### PRINT LIST (size %d )BY HAND ##### \n", list->size );
    node_t * pointer = list->head;
    printf("Head: %s\n", node_key(pointer));
    pointer = pointer->next;
    printf("Head->next : %s\n", node_key(pointer));
    pointer = pointer->next;
    printf("Head-> next -> next : %s\n", node_key(pointer));
    pointer = pointer->next;
    printf("Head-> next -> next : %s\n", node_key(pointer));
    pointer = pointer->next;
    printf("Head-> next -> next -> next : %s\n", node_key(pointer));
    pointer = pointer->next;
    printf("Head-> next -> next -> next -> next: %s\n", node_key(pointer));
    printf(" NOW TAIL ---------: \n");
    pointer = list->tail;
    printf("Tail: %s\n", node_key(pointer));
           pointer = pointer->prev;

    printf("Tail->prev : %s\n", node_key(pointer));
    pointer = pointer->prev;

    printf("Tail-> prev -> prev : %s\n", node_key(pointer));
    pointer = pointer->prev;

    printf("Tail-> prev -> prev : %s\n", node_key(pointer));
           pointer = pointer->prev;

    printf("Tail-> prev -> prev -> prev : %s\n", node_key(pointer));
           pointer = pointer->prev;

    printf("Tail-> prev -> prev -> prev -> prev: %s\n", node_key(pointer))  ;
    pointer = list->tail;

    printf("Tail-> next: %s\n", node_key(pointer->next))  ;
    printf("Tail-> next next: %s\n", node_key(pointer->next->next))  ;
  //  list_print(list);

   /* list_add(list, entryD);
    list_print(list);
    
    list_add(list, entryD);
    list_print(list);
    
    list_add(list, entryA);
    list_print(list);*/
    
    
    
    struct tuple_t * tupleTempalte = tuple_dup(tupleA);
    tupleTempalte->tuple[0] = strdup("Aane");
    tupleTempalte->tuple[1] = strdup("NULL");
    tupleTempalte->tuple[2] = strdup("NULL");

    struct entry_t * matchedEntry = list_get(list, tupleTempalte);
    
    printf("MatchedEntry with key %s has key %s\n", tupleTempalte->tuple[0],  entry_key(matchedEntry));
    
    printf("BEFORE REMOVE LIST IS \n");
    list_print(list);
    list_remove(list, tupleTempalte);
    printf("AFTER REMOVE LIST IS \n");
    list_print(list);

    list_destroy(list);
    list_print(list);

    

   // list_print(list);

    
    
    return 0;
}

char ** createTupleElements(char* key, char* e1, char * e2) {
    char ** ElementsForTuple = (char**) malloc(sizeof(char*)*3);
    
    ElementsForTuple[0] = strdup(key);
    ElementsForTuple[1] = strdup(e1);
    ElementsForTuple[2] = strdup(e2);

    return ElementsForTuple;
}



