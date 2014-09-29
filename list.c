    //
    //  list.c
    //  SD_2014
    //
    //  Created by Bruno Mata on 22/09/14.
    //
    //

    #include "list.h"
    #include "list-private.h"
    #include "tuple-private.h"
    #include <stdlib.h>
    #include <string.h>

    /* Cria uma nova lista. Em caso de erro, retorna NULL.
     */
    struct list_t *list_create() {
        struct list_t * newList = (struct list_t*) malloc (sizeof(struct list_t));
        
        if ( newList != NULL  ) {
       //     printf("list_create - sucess on malloc - will initialize attributes now\n");
            newList->size = 0;
            newList->head = NULL;
            newList->tail = NULL;
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
        
        while ( current != NULL && numberOfFreedNodes < numberOfNodes-1 ) {
            if ( current->prev != NULL ) {
                node_destroy(current->prev);
                numberOfFreedNodes++;
            }
            current = current->next;
        }
        list->size = 0;
        list->head = NULL;
        list->tail = NULL;
        free(list);
        
        return numberOfFreedNodes == numberOfNodes ? 0 : -1;
    }


    /* Adiciona uma entry na lista. Como a lista deve ser ordenada,
     * a nova entry deve ser colocada no local correto.
     *  A ordenação da lista deve ser por ordem decrescente das 
     * chaves alfanuméricas dos valores inseridos.
     * Retorna 0 (OK) ou -1 (erro)
     */
    int list_add(struct list_t *list, struct entry_t *entry) {
        //flag to return task success
        unsigned int taskSucess = -1;
        //safety check
        if ( list == NULL || entry == NULL)
            return taskSucess;
        
        //creates new node empty
        node_t * newNode = (node_t *) malloc (sizeof(node_t));
        //safety check
        if ( newNode == NULL)
            return taskSucess;
        
        //sets the entry
        newNode->entry = entry_dup(entry);
        //pointer iterate
        node_t * currentNode = list_head(list);
        //flag to indicate if the new node must be inserted before (0) or after (1)
        unsigned int positionToPut = 1;
        //while the new entry is not frist (DESC alfabethic order)
        unsigned int nodesToCheck = list_size(list);
        //searches for the node proper node to choose where to be inserted.
        while ( positionToPut != 0 && nodesToCheck > 0 ) {
            //if new node has key higher than current node, it must me put before it
            if ( entry_keys_compare(node_entry(newNode), node_entry(currentNode)) > 0 ) {
                positionToPut = 0;
            }
            else {
                //moves forward, unless this was the last node to check
                currentNode = nodesToCheck == 1 ? currentNode : currentNode->next;
            }
            nodesToCheck--;
        }
        //puts newNode before or after currentNode (defined by positionToPut)
        taskSucess = list_put_node(list, newNode, currentNode, positionToPut );
        //increments the number of elements of the list if newNode was added.
        if ( taskSucess == 0 )
            list->size++;
        
        //returns task success indication (0/1)
        return taskSucess;
    }

    /* Elimina da lista um elemento (tuplo) de acordo com o padrão
     * tup_template.
     * Retorna 0 (OK) ou -1 (erro)
     */
    int list_remove(struct list_t *list, struct tuple_t *tup_template) {
        //flag to return task success
        int taskSuccess = -1;
        //gets a node to remove, that matches tup_template.
        node_t * nodeToRemove = list_get_node(list, tup_template);
        
        if ( nodeToRemove == NULL )
            return taskSuccess;
        
        if ( list_size(list) == 1 ) {
            node_destroy(nodeToRemove);
            list->head = NULL;
            list->tail = NULL;
        }
        else {
            //nodeA is the prev of aNode
            node_t * nodeBefore = nodeToRemove->prev;
            //node D is the next of aNode
            node_t * nodeAfter = nodeToRemove->next;
            
            //all the redifining happens...
            nodeBefore->next = nodeAfter;
            nodeAfter->prev = nodeBefore;
            // if nodeB was now added before nodeC which was the head,
            // nodeB becames the list->head and tail->next points to nodeB.
            if ( list->head == nodeToRemove ) {
                list->head = nodeAfter;
                list->tail->next = nodeAfter;
            }
            // if nodeC was now added after nodeB which was the tail,
            // nodeC becames the list->tail and head->prev points to nodeC.
            if ( list->tail == nodeToRemove ) {
                list->tail = nodeBefore;
                list->head->prev = nodeBefore;
            }
            
            //by last it destroys the node.
           node_destroy(nodeToRemove);
            
            list_size_dec(list);
        }
        
        
        return taskSuccess;
    }

    /*
     * Compare the keys of two entries with strcmp,
     * following the parameters same order.
     */
    int entry_keys_compare(struct entry_t * entryA, struct entry_t * entryB ) {
        return strcmp(entry_key(entryA), entry_key(entryB));
    }


    /*
     * Puts a node between two other nodes: first and second into a list
     * whose content is entry. nodesToCheck is a iteration flag to know if
     * there is need to rearrange the head or tail of the list.
     * Returns 0 if OK, -1 otherwise.
     */
    int list_put_node(struct list_t* list,  node_t * newNode, node_t* aNode, int beforeOrAfter) {
        
        //safety check
        if ( list == NULL || newNode == NULL ||  (aNode == NULL && list_size(list) > 0) ) {
            perror("Error: put_none with list or newNode or relative node null\n");
            return -1;
        }
    
        //if list is empty...
        if ( aNode == NULL && list_size(list) <= 0 ) {
            newNode->next = newNode;
            newNode->prev = newNode;
            list->head = newNode;
            list->tail = newNode;
        }
        else {
            // at this point the list has size >=1 and will add one now.
            // The idea is: aNode has a previous node: nodeA; and a next node: nodeD.
            // the newNode and aNode will be nodeB and nodeC, depending if newNode
            // has to be inserted before or after aNode.
            // If its to be inserted before, nodeB=newNode and nodeC=newNode, way around otherwise.
            
            //nodeA is the prev of aNode
            node_t * nodeA = aNode->prev;
            //node D is the next of aNode
            node_t * nodeD = aNode->next;
            //nodeB is the newNode if is to be added before aNode, otherwise its aNode
            node_t * nodeB = beforeOrAfter == 0 ? newNode : aNode;
            //if nodeB is the new node, nodeC is aNode, otherwise it is newNode
            node_t * nodeC = nodeB == newNode ? aNode : newNode;
            
            //all the redifining happens...
            nodeA->next = nodeB;
            nodeB->prev = nodeA;
            nodeB->next = nodeC;
            nodeC->prev = nodeB;
            nodeC->next = nodeD;
            nodeD->prev = nodeC;
            
            // if nodeB was now added before nodeC which was the head,
            // nodeB becames the list->head and tail->next points to nodeB.
            if ( nodeB == newNode && list->head == nodeC ) {
                list->head = nodeB;
                list->tail->next = nodeB;
            }
            
            // if nodeC was now added after nodeB which was the tail,
            // nodeC becames the list->tail and head->prev points to nodeC.
            if ( nodeC == newNode && list->tail == nodeB ) {
                list->tail = nodeC;
                list->head->prev = nodeC;
            }
        }
    
    //return with success
    return 0;
}


    /*
     * Method that cheks if a certain node matches a template.
     * Returns 1 if it matches, 0 otherwise.
     */
    int node_matches_template(node_t * node, struct tuple_t* template ) {
        return tuple_matches_template(entry_value(node_entry(node)), template);
    }

    /* Obtem um elemento (tuplo) da lista de acordo com o padrão
     * tup_template.
     * Retorna a referencia do elemento na lista (ou seja, uma alteração
     * implica alterar o elemento na lista).
     */
    struct entry_t *list_get(struct list_t *list, struct tuple_t *tup_template) {
        //gets the node that matches the tup_template
        node_t * matchedNode = list_get_node(list, tup_template);
        //if there is a matched node returns its entry, null otherwise
        return matchedNode == NULL ? NULL : node_entry(matchedNode);
    }


    /*
     * Method that searches for a matching node with tup_template
     * and returns it if exists or NULL if it doesnt.
     */
    node_t * list_get_node(struct list_t *list, struct tuple_t *tup_template) {
        //safety check
        if ( list == NULL || list_isEmpty(list) || tup_template == NULL )
            return NULL;
        
        //pointer node to iterare
        node_t * matchedNode = list->head;
        //number of nodes to check matching
        unsigned int nodesToCheck = list_size(list);
        // flag to let then know if there was a match (true = 1, false = 0)
        unsigned int thereWasAMatch = 0;
        //It will move forward until it currentNode matches the template
        while ( nodesToCheck-- > 0 ) {
            if ( node_matches_template(matchedNode, tup_template) ) {
                //since thereWasAMatch, there are no need to check more nodes
                nodesToCheck = 0;
                thereWasAMatch = 1;
            }
            else {
                //since the currentNode didnt match it moves forward
                matchedNode = matchedNode->prev;
            }
        }
        
        //if there was match it returns the entry of the matchedNode, NULL otherwise.
        return thereWasAMatch ? matchedNode : NULL;
    }



    /* Retorna o tamanho (numero de elementos) da lista
     * Retorna -1 em caso de erro.
     */
    int list_size(struct list_t *list) {
        return list->size;
    }

    /*
     * Returns 1 (true) if list is empty, 0 its not empty.
     */
    int list_isEmpty(struct list_t* list) {
        return list_size(list) == 0;
    }

