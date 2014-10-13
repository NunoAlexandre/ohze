//
//  table.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 09/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "table.h"
#include "table-private.h"
#include "list.h"
#include "tuple.h"
#include "tuple-private.h"

/* Função para criar/inicializar uma nova tabela hash, com n
 * linhas(n = módulo da função hash)
 */
struct table_t *table_create(int n) {
    table_t * newTable = ( table_t* ) malloc ( sizeof ( table_t ) );
    
    if ( newTable ) {
        //should I do this malloc and if yes should check if null?
        newTable->bucket =(struct list_t**) malloc ( (sizeof (struct list_t*) * n) );
        newTable->size = n;
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
        list_destroy(table->bucket[i]);
    }
    
    free(table->bucket);
    free(table);
}

/* Função para adicionar um tuplo na tabela.
 * Lembrar que num espaço de tuplos podem existir tuplos iguais.
 * Devolve 0 (ok) ou -1 (out of memory, outros erros)
 */
int table_put(struct table_t *table, struct tuple_t *tuple) {
    int slot_index = table_slot_index(table, tuple_key(tuple));
    
    if ( slot_index == -1)
        return -1;
    
    int taskSuccess = -1;
    
    struct list_t * target_list = table_slot_list(table, slot_index);
    //if list is already created for slot_index
    if ( target_list != NULL ) {
        taskSuccess = list_add(target_list, entry_create(tuple));
    }
    else {
        //list is empty
        table->bucket[slot_index] = list_create();
        taskSuccess = list_add(table->bucket[slot_index], entry_create(tuple));
    }
    
    
    return taskSuccess;
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
struct list_t *table_get(struct table_t *table, struct tuple_t *tup_template, int keep_tuples, int one_or_all);

/* Função para remover um ou todos os tuplos da tabela que estejam
 * de acordo com o template tup_template, e libertar a memória
 * utilizada no armazenamento desse(s) tuplo(s).
 * O argumento one_or_all indica se a função deve remover um ou
 * todos os tuplos que estejam de acordo com o template.
 * Devolve: 0 (ok), -1 (nenhum tuplo encontrado; outros erros)
 */
int table_del(struct table_t *table, struct tuple_t *tup_template, int one_or_all);

/* Devolve o número de elementos na tabela.
 */
int table_size(struct table_t *table) {    
    return table != NULL ? table->size : -1;
}





/************************* Table-private implementation *****************/

struct list_t * table_slot_list ( table_t * table, int index ) {
    return table->bucket[index];
}

int table_slot_index ( table_t * table, char * key ) {
    //gets the hascode of key
    int hashcode = table_hashcode(table, key);
    //if the hascode is not -1 (error flag) it returns the mod hashcode table_size
    return hashcode != -1 ? hashcode % table_size(table) : -1;
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
