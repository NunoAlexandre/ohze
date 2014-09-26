//
//  list.c
//  SD_2014
//
//  Created by Bruno Mata on 22/09/14.
//
//

#include "list.h"
#include "list-private.h"
#include <stdlib.h>
#include <string.h>

/* Cria uma nova lista. Em caso de erro, retorna NULL.
 */
struct list_t *list_create() {
    struct list_t * newList = (struct list_t*) malloc (sizeof(struct list_t));
    
    if ( newList != NULL  ) {
        printf("list_create - sucess on malloc - will initialize attributes now\n");
        newList->size = 0;
        newList->head = (struct node_t*) malloc (sizeof(node_t));
        newList->tail = (struct node_t*) malloc (sizeof(node_t));
    }
    
    return newList;
}

/* Elimina uma lista, libertando *toda* a memoria utilizada pela
 * lista.
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_destroy(struct list_t *list) {
    //current node: starts at the head node
    node_t * current = list->head;
    //the number of freed nodes: useful to resolve return value
    int numberOfFreedNodes = 0;
    //the number of nodes the list has
    int numberOfNodes = list->size;
    
    while ( current != NULL && numberOfFreedNodes < numberOfNodes ) {
        if ( current->prev != NULL ) {
            free(current->prev);
            numberOfFreedNodes++;
        }
        current = current->next;
    }
    list->size = 0;
    free(list);
    
    return numberOfFreedNodes == numberOfNodes ? 0 : -1;
}

/*
 * Puts a node between two other nodes: first and second into a list 
 * whose content is entry. nodesToCheck is a iteration flag to know if
 * there is need to rearrange the head or tail of the list.
 * Returns 0 if OK, -1 otherwise.
 */
int putNodeBetween(node_t * first, node_t* second,
                    struct list_t * list, struct entry_t* entry, int nodesToCheck)
{
    //sucess flag
    int nodeWasAdded = -1;
    //initializes the new node
    node_t * newNode = node_create(first, second, entry_create(entry->value));
    nodeWasAdded = newNode != NULL ? 0 : -1;
    if ( nodeWasAdded == 0 ) {
        first->next =newNode;
        second->prev = newNode;
        //rearranges head and tail if needed
        if ( nodesToCheck == list->size ) list->head = newNode;
        else if ( nodesToCheck == 0) list->tail = newNode;
    }
    return nodeWasAdded;
}

/* Adiciona uma entry na lista. Como a lista deve ser ordenada,
 * a nova entry deve ser colocada no local correto.
 *  A ordenação da lista deve ser por ordem decrescente das 
 * chaves alfanuméricas dos valores inseridos.
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_add(struct list_t *list, struct entry_t *entry) {
    int nodeWasAdded = -1;
    if ( list->size == 0) {
        node_t * newNode = (node_t*) malloc (sizeof(node_t));
        newNode = node_create(newNode, newNode, entry);
        if ( newNode != NULL) {
            list->head = newNode;
            list->tail = newNode;
            list->size++;
            nodeWasAdded = 0;
        }
    }
    else {
        int nodesToCheck = list->size;
        node_t * currentNode = list->head;
        char * entryKeyToAdd = getEntryKey(entry);
        while (currentNode!= NULL && nodesToCheck > 0) {
            char * currentNodeKey = getEntryKey(currentNode->entry);
            if ( strcmp(entryKeyToAdd, currentNodeKey) >= 0 ) {
                nodeWasAdded = putNodeBetween(currentNode->prev, currentNode, list, entry, nodesToCheck);
                nodesToCheck = 0;
                list->size++;
                
            }
            nodesToCheck--;
            currentNode = currentNode->next;
            
        }
    }
    
    return nodeWasAdded;
}

/* Elimina da lista um elemento (tuplo) de acordo com o padrão
 * tup_template.
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_remove(struct list_t *list, struct tuple_t *tup_template) {
    int didRemove = -1;
    if ( list == NULL || tuple_size(list->tuple) != tuple_size(tup_template)) {
        return didRemove;
    }
        
    node_t *current = list->head;
    int nodesToCheck = list->size;
    int tupleElementsNumber = tuple_size(list->tuple);
    
    while ( current != NULL && nodesToCheck > 0 ) {
        for ( int i = 0; i < tupleElementsNumber; i++) {
            if ( strcmp(current->entry->value->tuple[i], tuple_template->tuple[i], 
        }
    }

}

/* Obtem um elemento (tuplo) da lista de acordo com o padrão
 * tup_template.
 * Retorna a referencia do elemento na lista (ou seja, uma alteração
 * implica alterar o elemento na lista).
 */
struct entry_t *list_get(struct list_t *list, struct tuple_t *tup_template);

/* Retorna o tamanho (numero de elementos) da lista
 * Retorna -1 em caso de erro.
 */
int list_size(struct list_t *list) {
    return list->size;
}
