//
//  entry.c
//  SD_2014
//
//  Created by Grupo SD015 on 22/09/14.
//
//

#include "tuple.h"
#include "tuple-private.h"
#include "entry.h"
#include <stdlib.h>
#include <string.h> // memcpy
#include "message-private.h"
#include <stdio.h>
#include <assert.h>

/* Função que cria um novo par chave-valor (isto é, que inicializa
 * a estrutura e aloca a memória necessária), recebendo um timestamp a atribuir.
 */
struct entry_t * entry_create2(struct tuple_t *tuple, long long timestamp ) {
    struct entry_t *newEntry = (struct entry_t*) malloc(sizeof(struct entry_t));
    if ( newEntry != NULL ) {
        newEntry->timestamp = timestamp;
        newEntry->value = tuple;
    }
    return newEntry;
}

/* Função que cria um novo par chave-valor (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 */
struct entry_t *entry_create(struct tuple_t *tuple) {
    return entry_create2(tuple, 0);
}



/* Função que destroi um par chave-valor e liberta toda a memoria.
 */
void entry_destroy(struct entry_t *entry) {
    if ( entry != NULL) {
        tuple_destroy(entry->value);
    }
    free(entry);
}

/* Funcao que duplica um par chave-valor. */
struct entry_t *entry_dup(struct entry_t *entry){
    if ( entry == NULL || entry->value == NULL )
        return NULL;
    
    //if entry is valid
    return  entry_create(tuple_dup(entry->value));
}

/**********  Implementation of entry-private.h   ***********/

/*
 * Returns the key of a given entry.
 */
char * entry_key (struct entry_t * entry) {
    return tuple_key(entry->value);
}

/*
 * Returns the value (tuple) of a given entry.
 */
struct tuple_t * entry_value ( struct entry_t * entry ) {
    return entry->value;
}

long long entry_timestamp (struct entry_t * entry ) {
    if ( entry == NULL )
        return -1;
    
    return entry->timestamp;
}

int entry_size_bytes ( struct entry_t * entry) {
    
    if ( entry == NULL || entry->value == NULL)
        return -1;

    return TIMESTAMP_SIZE + tuple_size_bytes(entry->value);
}


int entry_serialize(struct entry_t * entry, char **serialized_entry) {
    //safety checks
    if ( entry == NULL || entry->value == NULL )
        return -1;
    
    //the tuple serialized (to buffer)
    char *serialized_tuple = NULL;
    //serializes the tuple and gets its size
    int serialized_tuple_size = tuple_serialize(entry_value(entry), &serialized_tuple);
    

    //size (bytes) of the serialized entry (to buffer)
    int serialized_entry_size = TIMESTAMP_SIZE + serialized_tuple_size;
    //offset to serialized_entry
    int offset = 0;
    
    //allocs the needed space for the buffer
    serialized_entry[0] = (char*) malloc( serialized_entry_size );
    
    //converts the timestamp (long long) to network format
    long long timestamp_to_network = swap_bytes_64(entry_timestamp(entry));
    //moves the timestamp to the buffer
    memcpy(serialized_entry[0]+offset, &timestamp_to_network, TIMESTAMP_SIZE);
    //moves offset
    offset+=TIMESTAMP_SIZE;
    
    //adds the serialized tuple to the serialized entry buffer
    memcpy(serialized_entry[0]+offset, serialized_tuple, serialized_tuple_size);
    offset+=serialized_tuple_size;
    
    assert( serialized_entry_size == offset);
    
    return serialized_entry_size;
}

struct entry_t *entry_deserialize(char *buffer, int buffer_size) {
    
    //safety checks
    if ( buffer == NULL || buffer_size == -1 )
        return NULL;
    
    //creates an empty entry build from the buffer
    //offset
    int offset = 0;
    
    //gets the timestamp
    long long timestamp_network = 0;
    //sets its value from the buffer
    memcpy(&timestamp_network, buffer+offset, TIMESTAMP_SIZE);
    
    //moves the offset
    offset+=TIMESTAMP_SIZE;
    
    //saves it to entry
    long long timestamp_host = swap_bytes_64 ( timestamp_network );
    
    //gets the tuple
    struct tuple_t * tuple = tuple_deserialize(buffer+offset, buffer_size-offset);
    
    //checks
    assert ( tuple_size_bytes(tuple) == buffer_size-offset );
    //moves the offset
    offset+= tuple_size_bytes(tuple);
    //checks
    assert ( buffer_size == offset );
    
    //finally creates the entry with the elements retrived from the buffer
    struct entry_t * entry = entry_create2(tuple, timestamp_host);
    
    return entry;
}

