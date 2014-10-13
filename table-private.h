//
//  table-private.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 09/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#ifndef SD15_Product_table_private_h
#define SD15_Product_table_private_h

#include "list.h"
#include <string.h>
#include <stdio.h>


typedef struct table_t {
    //the size of the table: numbero of slots
    unsigned int size;
    //a bucket of <size> slots
    struct list_t ** bucket;
} table_t;


int table_slot_index ( table_t * table, char * key );

/*
 * Method that gets a table and a tuple key and returns the proper table index.
 */
int table_hashcode (table_t * table, char * key);


struct list_t * table_slot_list ( table_t * table, int index );

#endif
