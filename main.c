//
//  main.c
//  SD
//
//  Created by Bruno Mata on 22/09/14.
//  Copyright (c) 2014 Bruno Mata. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tuple.h"
#include "list.h"
#include "list-private.h"

int main(int argc, const char * argv[]) {

    int tupleDim = 3;
    char* tupleA[tupleDim];
    
    tupleA[0] = (char *) malloc (sizeof("primeira"));
    tupleA[1] = (char *) malloc (sizeof("segunda"));
    tupleA[2] = (char *) malloc (sizeof("terceira"));

    strcpy(tupleA[0], "primeira");
    strcpy(tupleA[1], "segunda");
    strcpy(tupleA[2], "terceira");
    
    struct tuple_t * myTuple = tuple_create2(tupleDim, tupleA);
    
    struct tuple_t *copiedTuple = tuple_dup(myTuple);
    
   
    printf("First Tuple memory is %p and copied is %p \n", myTuple, copiedTuple);
    printf("1. First Tuple - first element is '%s' \n", *myTuple->tuple);
    printf("1. Second Tuple - first element is '%s' \n", *copiedTuple->tuple);
    strcpy(myTuple->tuple[0], "A0");
    printf("2. First Tuple - first element is '%s' \n", *myTuple->tuple);
    printf("2. Second Tuple - first element is '%s' \n", *copiedTuple->tuple);
    
    char * firstTupleValue = myTuple->tuple[0];

    printf("1. Pointer to First tuple value is '%s' \n", firstTupleValue);
   // tuple_destroy(myTuple);
    printf("2. Pointer to First tuple (destryoed) value is '%s' \n", firstTupleValue);
    
    
    struct list_t * myList = list_create();
    printf("1. My list has size : %d\n", list_size(myList));
    list_add(myList, entry_create(myTuple));
    printf("2. My list with one added has size : %d\n", list_size(myList));
    copiedTuple->tuple[0] = "A1";
    list_add(myList, entry_create(copiedTuple));
    
    struct tuple_t *thirdTuple = tuple_dup(copiedTuple);
    struct entry_t * thirdTupleEntry = entry_create(thirdTuple);
    thirdTupleEntry->value->tuple[0] = "A2";
    list_add(myList, thirdTupleEntry);
    list_add(myList, thirdTupleEntry);
    printf("3. My list with three added has size : %d\n", list_size(myList));
    
    printf("1. First element key is %s and second element key is %s and third element key is %s and tail is %s\n", getEntryKey(myList->head->entry), getEntryKey(myList->head->next->entry), getEntryKey(myList->head->next->next->entry) , getEntryKey(myList->tail->entry));
    list_destroy(myList);
    printf("2. My list destroyed has size : %d\n", list_size(myList));
    
    
    return 0;
}
