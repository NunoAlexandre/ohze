//
//  table-private.h
//  SD15-Product
//
//  Created by Grupo SD015 on 09/10/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#ifndef SD15_Product_table_private_h
#define SD15_Product_table_private_h

#include "list.h"
#include "table.h"
#include <string.h>
#include <stdio.h>

#define TABLE_DIMENSION 12


typedef struct table_t {
    //the size of the table: numbero of slots
    unsigned int size;
    //a bucket of <size> slots
    struct list_t ** bucket;
} table_t;


int table_put_entry(struct table_t *table, struct entry_t *entry);

void table_print( struct table_t * table );
int table_slot_index ( table_t * table, char * key );

struct list_t *table_get_entries(struct table_t *table, long long timestamp, int keep_tuples, int one_or_all);

struct list_t *table_get_by(struct table_t *table, void * search_element, int get_criterion, int keep_tuples, int one_or_all);

/*
 * Method that gets a table and a tuple key and returns the proper table index.
 */
int table_hashcode (table_t * table, char * key);


struct list_t * table_slot_list ( table_t * table, int index );

int table_slots ( struct table_t * table );

int table_get_array(struct table_t *table, struct tuple_t *tup_template, int whatToDOWithTheNodes, int one_or_all, struct tuple_t *** matching_tuples);

#endif
