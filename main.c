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

int main(int argc, const char * argv[]) {

    int tupleDim = 3;
    char* tupleA[tupleDim];
    
    tupleA[0] = (char *) malloc (sizeof("primeira"));
    tupleA[1] = (char *) malloc (sizeof("primeira"));
    strcpy(tupleA[0], "primeira");
    strcpy(tupleA[1], "segunda");
    
    struct tuple_t * myTuple = tuple_create2(tupleDim, tupleA);
    
    struct tuple_t *copiedTuple = tuple_dup(myTuple);
    
   
    printf("First Tuple memory is %p and copied is %p \n", myTuple, copiedTuple);
    printf("1. First Tuple - first element is '%s' \n", *myTuple->tuple);
    printf("1. Second Tuple - first element is '%s' \n", *copiedTuple->tuple);
    strcpy(myTuple->tuple[0], "capum");
    printf("2. First Tuple - first element is '%s' \n", *myTuple->tuple);
    printf("2. Second Tuple - first element is '%s' \n", *copiedTuple->tuple);
    
    char * firstTupleValue = myTuple->tuple[0];

    printf("1. Pointer to First tuple value is '%s' \n", firstTupleValue);
    tuple_destroy(myTuple);
    printf("2. Pointer to First tuple (destryoed) value is '%s' \n", firstTupleValue);
    
    return 0;
}
