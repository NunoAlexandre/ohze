//
//  table.c
//  SD15-Product
//
//  Created by Grupo SD015 on 09/10/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "table.h"
#include "table-private.h"
#include "list.h"
#include "list-private.h"
#include "tuple.h"
#include "tuple-private.h"
#include "general_utils.h"

/* Função para criar/inicializar uma nova tabela hash, com n
 * linhas(n = módulo da função hash)
 */
struct table_t *table_create(int n) {
    table_t * newTable = ( table_t* ) malloc ( sizeof ( table_t ) );
    
    if ( newTable != NULL ) {
        //should I do this malloc and if yes should check if null?
        newTable->bucket =(struct list_t**) malloc ( (sizeof (struct list_t*) * n) );
        newTable->size = n;
        int i = 0;
        for (i = 0; i < n; i++)
            newTable->bucket[i] = list_create();
    }
    
    return newTable;
}

/* Eliminar/desalocar toda a memória
 */
void table_destroy(struct table_t *table) {
    if ( table == NULL || table->bucket == NULL )
        return;
    
    int i;
    for (i = 0; i < table->size; i++ ) {
        if ( table->bucket[i] != NULL )
            list_destroy(table->bucket[i]);
    }
    
    free(table->bucket);
    free(table);
}


/* Função para adicionar uma entrada à tabela.
 * Devolve 0 (ok) ou -1 (out of memory, outros erros)
 */
int table_put_entry(struct table_t *table, struct entry_t *entry) {
    int slot_index = table_slot_index(table, entry_key(entry));
    if ( slot_index == -1)
        return -1;
    
    int taskSuccess = -1;
    
    struct list_t * target_list = table_slot_list(table, slot_index);

    taskSuccess = list_add(target_list, entry);

    return taskSuccess;
}

/* Função para adicionar um tuplo na tabela.
 * Lembrar que num espaço de tuplos podem existir tuplos iguais.
 * Devolve 0 (ok) ou -1 (out of memory, outros erros)
 */
int table_put(struct table_t *table, struct tuple_t *tuple) {
    return table_put_entry(table, entry_create(tuple));
}


/* Função para obter um ou todos os tuplos da tabela que
 * estejam de acordo com o template tup_template.
 * Retorna uma lista contendo os tuplos.
 * O argumento keep_tuples indica se o(s) tuplo(s) encontrado(s)
 * deve(m) ser mantido(s) na tabela após a operação.
 * O argumento one_or_all indica se a função deve obter um ou todos
 * os tuplos que estejam de acordo com o template.
 * Em caso de erro, devolve NULL
 */
struct list_t *table_get(struct table_t *table, struct tuple_t *tup_template, int keep_tuples, int one_or_all) {
    
   return table_get_by(table, tup_template, GET_BY_TUPLE_MATCH, keep_tuples, one_or_all);
}

struct list_t *table_get_entries(struct table_t *table, long long timestamp, int keep_tuples, int one_or_all) {
    
    return table_get_by(table, &timestamp, GET_BY_TIME, keep_tuples, one_or_all);
}



/* Função para obter um ou todos os tuplos da tabela que
 * estejam de acordo com o template tup_template.
 * Retorna uma lista contendo os tuplos.
 * O argumento keep_tuples indica se o(s) tuplo(s) encontrado(s)
 * deve(m) ser mantido(s) na tabela após a operação.
 * O argumento one_or_all indica se a função deve obter um ou todos
 * os tuplos que estejam de acordo com o template.
 * Em caso de erro, devolve NULL
 */
struct list_t *table_get_by(struct table_t *table, void * search_element, int get_criterion, int keep_tuples, int one_or_all) {
    
    if ( table == NULL || search_element == NULL ) {
        puts("table == NULL");
        return NULL;
    }
    
    //gets the slot index where to search or -1 (must search on every slots) (if get_by_time is always -1)
    int slotIndex = get_criterion == GET_BY_TUPLE_MATCH ?
        table_slot_index(table, tuple_key(search_element)) : -1 ;
    
    //the list with all matching nodes that will then be returned
    struct list_t * allMatchingNodes = NULL;
    
    if ( slotIndex == -1) {
        //must have it own space where to add all the nodes found on every slots of the table.
        allMatchingNodes = list_create();
        //iterates over all slots of the table.
        int slotsToCheck = table_slots(table);
        int index =0;
        while ( slotsToCheck-- > 0 ) {
            //gets the list to search from
            struct list_t * list_to_search = table_slot_list(table, index);
            
            struct list_t * this_slot_matching_nodes = get_criterion == GET_BY_TUPLE_MATCH ?
                        list_matching_nodes(list_to_search, (struct tuple_t *) search_element, keep_tuples, one_or_all)
                        : list_entries_newer_than(list_to_search,  *((long long *) search_element), keep_tuples, one_or_all);
            
            long long reference_timestamp = get_criterion == GET_BY_TIME ? *((long long *) search_element) : 0;
            //moves all this_slot_matching_nodes to the matching_nodes list using list_add criterium
            // and not keeping the matching nodes at origin once this_slot_matching_nodes is temporary.
            int move_criterion = get_criterion == GET_BY_TIME ? MOVE_WITH_CRITERION_TIME : MOVE_WITH_CRITERION_KEY;
            list_move_nodes (this_slot_matching_nodes , allMatchingNodes , move_criterion, reference_timestamp, DONT_KEEP_AT_ORIGIN );
            
            //if its just to get one and list is not empty it found one so it stops
            if ( one_or_all == 1 && !list_isEmpty(allMatchingNodes) ) {
                slotsToCheck = 0;
            }
            index++;
        }
    }
    else {
        //this is the only slot list to search from
        struct list_t * list_to_search = table_slot_list(table, slotIndex);
        
        //once this slot was the only to be searched from,
        //allMatchingNodes is this table_slot_list matching nodes.
        allMatchingNodes = get_criterion == GET_BY_TUPLE_MATCH ?
            list_matching_nodes(list_to_search, (struct tuple_t *) search_element, keep_tuples, one_or_all)
            : list_entries_newer_than(list_to_search,  *((long long *) search_element), keep_tuples, one_or_all);
    }

    
    return allMatchingNodes;
}




int table_get_array(struct table_t *table, struct tuple_t *tup_template, 
    int whatToDOWithTheNodes, int one_or_all, struct tuple_t *** matching_tuples)
{

    //gets the matching nodes on a list_t
    struct list_t * matching_nodes = table_get(table, tup_template, whatToDOWithTheNodes, one_or_all);
    //gets the number of matching nodes
    int matching_tuples_num = list_size(matching_nodes);

    //since matching_nodes_num > 0 creates the array of tuples
    *matching_tuples = tuple_create_array(matching_tuples_num);

    int i =0;
    node_t * currentNode = list_head(matching_nodes);
    for (i = 0; i < matching_tuples_num; i++ ) {
        //saves the tuple
        (*matching_tuples)[i] = entry_value(node_entry(currentNode));
        //moves to list next node
        currentNode = currentNode->next;  
    }

    //in the end destroys the list to free memory
    //tuple_array_destroy(matching_tuples, matching_tuples_num );
    list_destroy(matching_nodes);

    //returns the array matching_tuples
    return matching_tuples_num;
}

void table_print( struct table_t * table )
{
    if (table == NULL || table->size == 0 )
        printf("Tabela vazia");
    else {
        int i = 0;
        for ( i = 0; i < table->size;  i++) {
            printf("Table > slot %d : \n", i );
            list_print(table_slot_list(table, i));
            printf("\n");
        }
    }
}

/* Função para remover um ou todos os tuplos da tabela que estejam
 * de acordo com o template tup_template, e libertar a memória
 * utilizada no armazenamento desse(s) tuplo(s).
 * O argumento one_or_all indica se a função deve remover um ou
 * todos os tuplos que estejam de acordo com o template.
 * Devolve: 0 (ok), -1 (nenhum tuplo encontrado; outros erros)
 */
int table_del(struct table_t *table, struct tuple_t *tup_template, int one_or_all) {
    //gets all the matching nodes at the table with tup_template, not keeping them at the origin slot.
    struct list_t * allMatchingNodes = table_get(table, tup_template, JUST_DELETE_NODES, one_or_all);
    
    //since we choosed to JUST_DELETE_NODES on get, allMatchingNodes must be empty
    return list_isEmpty(allMatchingNodes) ? SUCCEEDED : FAILED;
}

int table_slots ( struct table_t * table ) {
    return table != NULL ? table->size : -1;
}

/* Devolve o número de elementos na tabela.
 */
int table_size(struct table_t *table) {
    
    if ( table == NULL || table->bucket ==  NULL)
        return 0;
    
    int totalNumberOfElement = 0;
    int i =0;
    
    for ( i = 0; i < table_slots(table); i++) {
        totalNumberOfElement += list_size(table_slot_list(table, i));
    }
    
    return totalNumberOfElement;
}


/************************* Table-private implementation *****************/

struct list_t * table_slot_list ( table_t * table, int index ) {
    return table->bucket[index];
}

/*
 * Having a table and a string key it returns the index for it or -1 if key is null
 */
int table_slot_index ( table_t * table, char * key ) {
    //gets the hascode of key
    int hashcode = table_hashcode(table, key);
    //if the hascode is not -1 (error flag) it returns the mod hashcode table_slots
    return hashcode != -1 ? hashcode % table_slots(table) : -1;
}
/*
 * Method that gets a table and a tuple key and returns the proper table index.
 */
int table_hashcode (table_t * table, char * key) {
    if ( table == NULL || table->bucket == NULL
        || table->size == 0|| key == NULL || strlen(key) == 0 )
        return -1;
    
    //the result hashcode
    int hashcode = 0;
    
    unsigned long key_size = strlen(key);
    //if table size is <=5
    if ( key_size <= 5 ) {
        int i = 0;
        for ( i = 0; i < key_size; i++ ) {
            hashcode+= key[i];
        }
    }
    //if key size is >5 its sums the first two and last three chars of the key
    else {
        //sums ascsii of two firsts
        hashcode+= key[0] + key[1];
        //and the three last
        hashcode+= key[key_size-1] +  key[key_size-2] + key[key_size-3];
    }
    
    
    //now we have the hashcode so we just return it
    return hashcode;
}
