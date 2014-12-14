//
//  list.c
//  SD_2014
//
//  Created by Grupo SD015 on 22/09/14.
//
//

#include "list.h"
#include "list-private.h"
#include "tuple-private.h"
#include "entry-private.h"
#include "general_utils.h"
#include <stdlib.h>
#include <string.h>
#include "message-private.h"

/* Cria uma nova lista. Em caso de erro, retorna NULL.
 */
struct list_t *list_create() {
    struct list_t * newList = (struct list_t*) malloc (sizeof(struct list_t));
    
    if ( newList != NULL  ) {
        newList->size = 0;
        newList->head = NULL;
        newList->tail = NULL;
    }
    return newList;
}

/* Elimina uma lista, libertando *toda* a memoria utilizada pela
 * lista
 * Retorna 0 (OK) ou -1 (erro)
 */
 int list_destroy(struct list_t *list) {
    if ( list == NULL )
        return -1;
    //current node: starts at the head node
    node_t * current = list_head(list);
    //the number of freed nodes: useful to resolve return value
    int numberOfFreedNodes = 0;
    //the number of nodes the list has
    int numberOfNodes = list_size(list);
    
    while ( current != NULL && numberOfFreedNodes < numberOfNodes-1 ) {
        if ( current->prev != NULL ) {
            //if node is destroyed it increments the freed nodes number
            if ( node_destroy(current->prev)  == SUCCEEDED ) {
                numberOfFreedNodes++;
            }
        }
        current = current->next;
    }
    free(list);
    
    return numberOfFreedNodes == numberOfNodes ? 0 : -1;
}

// nodeToAdd fica antes do currentNode se...
int node_matches_criterion( node_t * nodeToAdd, node_t * currentNode, int criterion, long long reference_timestamp) {
    int matches = NO;
    if ( criterion == ADD_WITH_CRITERION_TIME ) {
        matches = (nodeToAdd->entry->timestamp < currentNode->entry->timestamp)
            && (nodeToAdd->entry->timestamp > reference_timestamp);
    }
    else if ( criterion == ADD_WITH_CRITERION_KEY )
        matches = entry_keys_compare(node_entry(nodeToAdd), node_entry(currentNode)) > 0 ;
    else
        matches = NO;
    
    return matches;
}


int  list_add_node (struct list_t *list, node_t * newNode, int add_criterion, long long reference_timestamp ) {

    //flag to return task success
    int taskSucess = -1;
    
    //safety check
    if ( newNode == NULL)
        return taskSucess;
    
    //pointer iterate

    node_t * currentNode = add_criterion ? list_head(list) : list_tail(list);

    //flag to indicate if the new node must be inserted before (0) or after (1)
    unsigned int positionToPut = 1;
    //while the new entry is not frist (DESC alfabethic order)
    unsigned int nodesToCheck = add_criterion ? list_size(list) : 0;
    //searches for the node proper node to choose where to be inserted.
    while ( positionToPut != 0 && nodesToCheck > 0 ) {
        //if new node has key higher than current node, it must me put before it
        if ( node_matches_criterion(newNode, currentNode, add_criterion, reference_timestamp)  ) {
                       positionToPut = 0;
        }
        else {
            //moves forward, unless this was the last node to check
            currentNode = nodesToCheck == 1 ? currentNode : currentNode->next;
        }
        nodesToCheck--;
    }
    //puts newNode before or after currentNode (defined by positionToPut)
    taskSucess = list_insert_node(list, newNode, currentNode, positionToPut );
    
    //returns task success indication (0/1)
    return taskSucess;
    
}



/* Adiciona uma entry na lista. Como a lista deve ser ordenada,
 * a nova entry deve ser colocada no local correto.
 *  A ordenação da lista deve ser por ordem decrescente das
 * chaves alfanuméricas dos valores inseridos.
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_add_with_criterion(struct list_t *list, struct entry_t *entry, int move_criterion, long long reference_timestamp) {
    
    //flag to return task success
    int taskSucess = FAILED;
    //safety check
    if ( list == NULL || entry == NULL )
        return taskSucess;
    
    //creates new node empty
    node_t * newNode = node_create(NULL, NULL, entry);
    //adds the nodes and saves task success
    taskSucess = list_add_node(list, newNode, move_criterion, reference_timestamp);
    //if something went wrong it frees the new node
    if ( taskSucess == FAILED) {
        node_destroy(newNode);
    }
    
    return taskSucess;
}

/* Adiciona uma entry na lista. Como a lista deve ser ordenada,
 * a nova entry deve ser colocada no local correto.
 *  A ordenação da lista deve ser por ordem decrescente das
 * chaves alfanuméricas dos valores inseridos.
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_add(struct list_t *list, struct entry_t *entry) {
    return list_add_with_criterion(list, entry, MOVE_WITH_CRITERION_KEY, 0 );
}

int list_remove_node (struct list_t * list, node_t * nodeToRemove, int mustDestroy ) {

    //safety check
    if ( list == NULL || nodeToRemove == NULL)
        return FAILED;
    
    //success flag, fail as a start since nothing was done yet
    int taskSuccess = FAILED;
    
    if ( list_size(list) == 1 ) {
        list->head = NULL;
        list->tail = NULL;
        list_size_dec(list);
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
        if ( list_head(list) == nodeToRemove ) {
            list->head = nodeAfter;
            list->tail->next = nodeAfter;
        }
        // if nodeC was now added after nodeB which was the tail,
        // nodeC becames the list->tail and head->prev points to nodeC.
        if ( list->tail == nodeToRemove ) {
            list->tail = nodeBefore;
            list->head->prev = nodeBefore;
        }
        
        //decrements the list size
        list_size_dec(list);
    }
    
    //by last, the task success depends on the node_destroy success or is
    // success by it self if node must not be destroyed
    if ( mustDestroy ) {
        entry_destroy(node_entry(nodeToRemove));
        taskSuccess = node_destroy(nodeToRemove);
    }
    else {
        taskSuccess = SUCCEEDED;
    }
    
    //returns the taskSucess
    return taskSuccess;
    
}
/* Elimina da lista um elemento (tuplo) de acordo com o padrão
 * tup_template.
 * Retorna 0 (OK) ou -1 (erro)
 */
 int list_remove(struct list_t *list, struct tuple_t *tup_template) {
    //as a start the taskSuccess is failed (nothing done yet)
    //safety check
    if ( list == NULL || tup_template == NULL)
        return FAILED;
    
    //searchs on the list 1 Node matching tup_template and
    //simply deletes it when finds it, returning null if succedded
    node_t * removedNode = list_get_one (list, tup_template, JUST_DELETE_NODES);
    
    //if the removedNode is null then it was removed successfully.
    return removedNode == NULL ? SUCCEEDED : FAILED;
}

/*
 * Method that cheks if a certain node matches a template.
 * Returns 1 if it matches, 0 otherwise.
 */
 int node_matches_template(node_t * node, struct tuple_t* template ) {
    return tuple_matches_template(entry_value(node_entry(node)), template);
}
/*
 * Method that cheks if a certain node matches a template.
 * Returns 1 if it matches, 0 otherwise.
 */
int node_newer_than(node_t * nodeA, long long timestamp ) {
    return entry_newer_than(node_entry(nodeA), timestamp);
}

/* Obtem um elemento (tuplo) da lista de acordo com o padrão
 * tup_template.
 * Retorna a referencia do elemento na lista (ou seja, uma alteração
 * implica alterar o elemento na lista).
 */
struct entry_t *list_get(struct list_t *list, struct tuple_t *tup_template) {
    //gets the node that matches the tup_template
    node_t * matchedNode = list_get_one(list, tup_template, KEEP_AT_ORIGIN);
    //if there is a matched node returns its entry, null otherwise
    return matchedNode == NULL ? NULL : node_entry(matchedNode);
}



/**********  Implementation of private-list.h  ***********/

/*
 * Creates a node having the prev and next node and its entry.
 */
 node_t * node_create( struct node_t * prev, struct node_t* next, struct entry_t * entry) {
    node_t * newNode = (node_t *) malloc ( sizeof(node_t));
    if ( newNode != NULL ) {
        newNode->prev =  prev;
        newNode->next = next;
        newNode->entry = entry;
    }
    
    return newNode;
}

/*
 * Creates an empty node - just allocates memory for the structure.
 */
 node_t * node_create_empty() {
    node_t * newNode = (node_t *) malloc ( sizeof(node_t) );
    return newNode;
}

/*
 * Duplicates a node.
 */
 node_t * node_dup(node_t* node) {
    if ( node == NULL || node->entry == NULL)
        return NULL;
    
    //if node to duplicate is valid
    return node_create(node->prev, node->next, node->entry);
}

/*
 * Destroyes a node.
 * Returns 0 (OK) or -1 (error)
 */
 int node_destroy (struct node_t* node ) {
    //safety checks in error case
    if ( node == NULL || node->entry == NULL )
        return FAILED;
    
    free(node);
    
    //success
    return SUCCEEDED;
}

/*
 * Method that checks if a certain tuple matches a template.
 * If the tuple and the template have different sizes they dont match.
 * If a field of the template is not null and is not equal to element
 * at same position of the tuple, they dont match.
 * Returns 1 (true) if they match, 0 otherwise.
 */
 int tuple_matches_template ( struct tuple_t * tuple , struct tuple_t * template ) {
    if ( tuple_size(tuple) != tuple_size(template) )
        return 0;
    
    //Flag to say if the tuple matchs the template.
    // if doesnt, matches turns 0 (false), otherwise it keeps 1.
    int matches = 1;
    // saves the number of checked elements of the tuple
    int iElement = 0;
    
    while ( matches && iElement < tuple_size(tuple) ) {
        //tuple iElement
        char * tupleElement = tuple_element(tuple, iElement);
        //template iElement
        char * templateElement = tuple_element(template, iElement);
        
        //if templateElement is not null but not equal to the tupleElement, doesnt match.
        if ( templateElement != NULL &&  tupleElement != NULL &&  strcmp(tupleElement, templateElement) != 0 )
            matches = 0;
        
        iElement++;
    }
    //if the match checking never failed it matches (0)
    return matches;
}

/*
 * Method thats inserts a node in the list, beforeOrAfter aNode.
 * If beforeOrAfter is 0, inserts before aNode,  if 1 adds it after it.
 * It can get aNode null if list_size(list) == 0.
 * Returns 0 in success case, -1 in error case.
 */
 int list_insert_node(struct list_t* list,  node_t * newNode, node_t* aNode, int beforeOrAfter) {

    //safety check
    if ( list == NULL || newNode == NULL ||  (aNode == NULL && list_size(list) > 0) ) {
        perror("Error: list_insert_node with list or newNode or relative node null\n");
        return SUCCEEDED;
    }
    
    //if list is empty...
    if ( aNode == NULL && list_isEmpty(list) ) {
        newNode->next = newNode;
        newNode->prev = newNode;
        list->head = newNode;
        list->tail = newNode;
    }
    else {
        //if there was not specified any relative aNode, if beforeOrAfter says before
        // the aNode is head, otherwise its tail.
       if ( aNode == NULL ) {
        aNode = beforeOrAfter == 0 ? list_head(list) : list_tail(list);
    }

        //if there is only one element at the list...
    if ( list_size(list) == 1 ) {
        node_t * nodeA = beforeOrAfter == 0 ? newNode : aNode;
        node_t * nodeB = nodeA == newNode ? aNode : newNode;

        nodeA->next = nodeB;
        nodeB->prev = nodeA;
        nodeA->prev = nodeB;
        nodeB->next = nodeA;

        list->head = nodeA;
        list->tail = nodeB;
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
        if ( nodeB == newNode && list_head(list) == aNode ) {
            list->head = nodeB;
            list->tail->next = nodeB;
        }

        // if nodeC was now added after nodeB which was the tail,
        // nodeC becames the list->tail and head->prev points to nodeC.
        if ( nodeC == newNode && list_tail(list) == aNode ) {
            list->tail = nodeC;
            list->head->prev = nodeC;
        }
    }
}



list_size_inc(list);
//return with success
return SUCCEEDED;
}

int list_insert_to_tail ( struct list_t * list, node_t* node) {
    return list_insert_node(list, node, NULL, 1);
}
int list_insert_to_head ( struct list_t * list, node_t* node) {
    return list_insert_node(list, node, NULL, 0);
}

/*
 * Moves all nodes from the fromList to the toList.
 * The way to move it depends on the mustMoveWithCriterium:
 * se 1/YES - uses list_add that uses its own insert criterium, 0/NO - moves to the tail of toList.
 */
 int list_move_nodes (struct  list_t * fromList, struct list_t * toList, int move_criterium, long long reference_timestamp, int whatToDoWithTheNode ) {

    if ( fromList == NULL || toList == NULL )
        return FAILED;
    
    int nodesToMove = list_size(fromList);
    node_t * currentNode = list_head(fromList);
    
    while ( nodesToMove-- > 0 ) {
        //moves currentNode to toList and if error (-1) returns it.
        if ( list_move_node(fromList, toList, currentNode, move_criterium, reference_timestamp, whatToDoWithTheNode ) == -1 ) {
            return FAILED;
        }
        //once we are iterating from head to tail the currentNode is now the new fromList head
        currentNode = whatToDoWithTheNode == KEEP_AT_ORIGIN ? currentNode->next : list_head(fromList);
    }
    
    // if it got to here there were no errors so it returns success
    return 0;
    
}


/*
 * Method that moves (not destroying ) a node fromList toList.
 * Returns 0 if moved successfully, -1 if error.
 */
 int list_move_node (struct  list_t * fromList, struct list_t * toList, node_t * node,
    int move_criterium, long long reference_timestamp, int whatToDoWithTheNode ) {

    //safety checks
    if ( fromList == NULL || toList == NULL || node == NULL)
        return FAILED;
    
    //if taskSuccess keeps 0 means all taks succeded
    int taskSuccess = 0;
    
    if ( whatToDoWithTheNode == KEEP_AT_ORIGIN ) {
        //if node must be kept at the fromList (origin) we use list_add that
        //adds to toList a new node with the entry of node
        taskSuccess+=list_add_with_criterion(toList, node_entry(node), move_criterium, reference_timestamp);
    }
    else if ( whatToDoWithTheNode == DONT_KEEP_AT_ORIGIN ) {
        //if it must be removed from the original list we only need to
        // remove it from the original list without destroying it.
        taskSuccess+= list_remove_node(fromList, node, NOT_DESTROY );
        //now the matchedNode is out of the list and can be inserted
        //or added (depending on mustMoveWithCritirion option ) to the matching nodes list
        taskSuccess+= list_add_node(toList, node, move_criterium, reference_timestamp);
    }
    else if ( whatToDoWithTheNode == JUST_DELETE_NODES ) {
        taskSuccess+= list_remove_node(fromList, node, MUST_DESTROY );
    }
    //returns the taskSuccess (0 ok, -1 error)
    return taskSuccess == 0 ? SUCCEEDED : FAILED;
}

struct list_t * list_matching_nodes (struct list_t *list, struct tuple_t *tup_template,
 int whatToDoWithTheNode, int getJustOne )
{
    //safety check
    if ( list_isEmpty(list) || tup_template == NULL )
        return NULL;
    
    
    //the list where to save all the matching nodes found on this list.
    struct list_t * matching_nodes = list_create();
    
    //pointer node to iterare
    node_t * matchedNode = list_head(list);
    //number of nodes to check matching
    unsigned int nodesToCheck = list_size(list);
    
    //It will move forward until it currentNode matches the template
    while ( nodesToCheck-- > 0 ) {

        //in case of a match and a node is removed
        node_t * nextNode = matchedNode->next;
        if ( node_matches_template(matchedNode, tup_template) ) {
            //if moves the matchedNode from list to matching_nodes
            // with adding criterion and matchedNode whatToDoWithTheNode or not.
            list_move_node(list, matching_nodes, matchedNode, MOVE_WITH_CRITERION_KEY, 0, whatToDoWithTheNode);
            
            //if it must get just one it stops to check.
            if ( getJustOne ) {
                nodesToCheck = 0;
            }
        }
        //since the currentNode didnt match it moves forward
        matchedNode = nextNode;
    }
    //if there was match it returns the entry of the matchedNode, NULL otherwise.
    return matching_nodes;
    
}

struct list_t * list_entries_newer_than (struct list_t *list, long long timestamp, int whatToDoWithTheNode, int getJustOne ) {
    //safety check
    if ( list_isEmpty(list) )
        return NULL;
    
    
    //the list where to save all the matching nodes found on this list.
    struct list_t * newer_entries = list_create();
    
    //pointer node to iterare
    node_t * currentNode = list_head(list);
    //number of nodes to check matching
    unsigned int nodesToCheck = list_size(list);
    
    //It will move forward until it currentNode matches the template
    while ( nodesToCheck-- > 0 ) {
        
        //in case of a match and a node is removed
        node_t * nextNode = currentNode->next;
        if ( node_newer_than(currentNode, timestamp) ) {
            //if moves the matchedNode from list to matching_nodes
            list_move_node(list, newer_entries, currentNode, MOVE_WITH_CRITERION_TIME, timestamp, whatToDoWithTheNode);
            
            //if it must get just one it stops to check.
            if ( getJustOne ) {
                nodesToCheck = 0;
            }
        }
        //since the currentNode didnt match it moves forward
        currentNode = nextNode;
    }
    //if there was match it returns the entry of the matchedNode, NULL otherwise.
    return newer_entries;
}



/*
 * Returns 1 (true) if list is empty, 0 its not empty.
 */
 int list_isEmpty(struct list_t* list) {
    return list == NULL || list_size(list) <= 0;
}

/* Retorna o tamanho (numero de elementos) da lista
 * Retorna -1 em caso de erro.
 */
 int list_size(struct list_t *list) {
    return list == NULL ? 0 : list->size;
}

/*
 * Method that returns the key of the node.
 * Delegates the request to the type of entry it uses.
 */
 char * node_key( node_t * node ) {
    return entry_key(node->entry);
}

/*
 * Method that returns the entry of a given node.
 */
 struct entry_t * node_entry(node_t* node) {
    return node->entry;
}

/*
 * Method that compares two entry keys with strcmp/2
 * on the same order than the parameters are received.
 */
 int entry_keys_compare(struct entry_t * entryA, struct entry_t * entryB ) {
    return strcasecmp(entry_key(entryA), entry_key(entryB));
}

/*
 * Method that increments +1 to the size of the the given list.
 */
 void list_size_inc(struct list_t * list) {
    list->size++;
}

/*
 * Method that decrements -1 to the size of the given list.
 */
 void list_size_dec( struct list_t * list) {
    list->size--;
}

/*
 * Method that returns the tail of the given list.
 */
 node_t * list_tail ( struct list_t* list) {
    return list->tail;
}

/*
 * Method that returns the head of the given list.
 */
 node_t * list_head(struct list_t * list ) {
    return list != NULL ? list->head : NULL;
}


node_t* list_get_one ( struct list_t * list, struct tuple_t * tup_template, int keepAtOrigin) {
    struct list_t * matching_nodes = list_matching_nodes(list, tup_template, keepAtOrigin, 1);
    return list_head(matching_nodes);
}
/*
 * Gets all the elements of the list that match tup_template
 */
 struct list_t * list_get_all ( struct list_t * list, struct tuple_t * tup_template, int mustRemove) {
    return list_matching_nodes(list, tup_template, mustRemove, 0);
}

/*
 * Method to print the given list.
 */
 void list_print ( struct list_t * list) {
    printf("# list_print > size %d : ", list_size(list));
    node_t * nodeToPrint = list_head(list);
    printf("list_print : list has size %d \n", list_size(list));
    int nodesToPrint = list->size;
    while ( nodesToPrint--  > 0 ) {
        message_print(message_create_with(OC_UPDATE, CT_ENTRY, nodeToPrint->entry));puts("");
       nodeToPrint = nodeToPrint->next;
   }   
}
