#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include "tuple.h"
//#include "entry.h"
#include "list.h"
#include "list-private.h"

/***********************************************************************************/
/* Lista vazia bem criada?                                                         */
/***********************************************************************************/
int testListaVazia() {
    struct list_t *list = list_create();
    
    int result = list != NULL && list_size(list) == 0;
    
    list_destroy(list);
    
    printf("  Módulo list, teste lista vazia: %s\n", result ? "passou" : "não passou");
    return result;
}

/***********************************************************************************/
/* Primeiro elemento bem inserido?                                                 */
/***********************************************************************************/
int testAddCabeca() {
    int result;
    struct list_t *list = list_create();
    
    char *tdata[3] = {"SD", "2014", "Fixe!"};
    struct tuple_t *t = tuple_create2(3, tdata);
    struct tuple_t *tdup = tuple_dup(t);
    
    struct entry_t *entry = entry_create(t);
    
    list_add(list, entry);
    
    result = (list_get(list, tdup) == entry) &&
    (list_size(list) == 1);
    
    tuple_destroy(tdup);
    list_destroy(list);
    
    printf("  Modulo list, teste adicionar cabeça: %s\n", result? "passou" : "não passou");
    return result;
}

/***********************************************************************************/
/* Múltiplos elementos bem inseridos e encontrados?                                */
/***********************************************************************************/
int testAddVarios() {
    int result, i;
    struct list_t *list = list_create();
    
    char *tdata[3] = {"       ", "2014", "Fixe!"};
    struct tuple_t *t = tuple_create2(3, tdata);
    struct tuple_t *tdups[1024];
    
    struct entry_t *entries[1024];
    
    for(i = 0; i < 1024; i++) {
        sprintf(t->tuple[0], "SD-%d", i);
        tdups[i] = tuple_dup(t);
        entries[i] = entry_create(tdups[i]);
        list_add(list,entries[i]);
    }
    
    assert(list_size(list) == 1024);
    result = (list_size(list) == 1024);
    
    for(i=0; i<1024; i++) {
        sprintf(t->tuple[0], "SD-%d", i);
        assert(list_get(list, tdups[i]) == entries[i]);
        result = result && (list_get(list, tdups[i]) == entries[i]);
    }
    
    tuple_destroy(t);
    list_destroy(list);
    
    printf("  Módulo list, teste adicionar vários: %s\n", result ? "passou" : "não passou");
    return result;
}

/***********************************************************************************/
/* Head bem removida?                                                              */
/***********************************************************************************/
int testRemoveCabeca() {
    int result, i;
    struct list_t *list = list_create();
    
    char *tdata[3] = {"   ", "2014", "Fixe!"};
    struct tuple_t *t = tuple_create2(3, tdata);
    struct tuple_t *tdups[4];
    
    struct entry_t *entries[4];
    
    sprintf(t->tuple[0], "abc");
    tdups[0] = tuple_dup(t);
    entries[0] = entry_create(tdups[0]);
    list_add(list, entries[0]);
    
    sprintf(t->tuple[0], "def");
    tdups[1] = tuple_dup(t);
    entries[1] = entry_create(tdups[1]);
    list_add(list, entries[1]);
    
    sprintf(t->tuple[0], "ghi");
    tdups[2] = tuple_dup(t);
    entries[2] = entry_create(tdups[2]);
    list_add(list, entries[2]);
    
    sprintf(t->tuple[0], "jkl");
    tdups[3] = tuple_dup(t);
    entries[3] = entry_create(tdups[3]);
    list_add(list, entries[3]);
    
    result = (list_remove(list, tdups[3]) == 0) &&
    (list_get(list, tdups[2]) == entries[2]) &&
    (list_get(list, tdups[1]) == entries[1]) &&
    (list_get(list, tdups[0]) == entries[0]) &&
    (list_size(list) == 3);
    
    tuple_destroy(t);
    list_destroy(list);
    
    printf("  Módulo list, teste remover cabeça: %s\n", result ? "passou" : "não passou");
    return result;
}

/***********************************************************************************/
/* Tail bem removida?                                                              */
/***********************************************************************************/
int testRemoveCauda() {
    int result, i;
    struct list_t *list = list_create();
    
    char *tdata[3] = {"   ", "2014", "Fixe!"};
    struct tuple_t *t = tuple_create2(3, tdata);
    struct tuple_t *tdups[4];
    
    struct entry_t *entries[4];
    
    sprintf(t->tuple[0], "abc");
    tdups[0] = tuple_dup(t);
    entries[0] = entry_create(tdups[0]);
    list_add(list, entries[0]);
    
    sprintf(t->tuple[0], "def");
    tdups[1] = tuple_dup(t);
    entries[1] = entry_create(tdups[1]);
    list_add(list, entries[1]);
    
    sprintf(t->tuple[0], "ghi");
    tdups[2] = tuple_dup(t);
    entries[2] = entry_create(tdups[2]);
    list_add(list, entries[2]);
    
    sprintf(t->tuple[0], "jkl");
    tdups[3] = tuple_dup(t);
    entries[3] = entry_create(tdups[3]);
    list_add(list, entries[3]);
    
    result = (list_remove(list, tdups[0]) == 0) &&
    (list_get(list, tdups[3]) == entries[3]) &&
    (list_get(list, tdups[1]) == entries[1]) &&
    (list_get(list, tdups[2]) == entries[2]) &&
    (list_size(list) == 3);
    
    tuple_destroy(t);
    list_destroy(list);
    
    printf("  Módulo list, teste remover cauda: %s\n", result ? "passou" : "não passou");
    return result;
}

/***********************************************************************************/
/* "Meios" bem removidos?                                                          */
/***********************************************************************************/
int testRemoveMeio() {
    int result, i;
    struct list_t *list = list_create();
    
    char *tdata[3] = {"   ", "2014", "Fixe!"};
    struct tuple_t *t = tuple_create2(3, tdata);
    struct tuple_t *tdups[4];
    
    struct entry_t *entries[4];
    
    sprintf(t->tuple[0], "abc");
    tdups[0] = tuple_dup(t);
    entries[0] = entry_create(tdups[0]);
    list_add(list, entries[0]);
    
    sprintf(t->tuple[0], "def");
    tdups[1] = tuple_dup(t);
    entries[1] = entry_create(tdups[1]);
    list_add(list, entries[1]);
    
    sprintf(t->tuple[0], "ghi");
    tdups[2] = tuple_dup(t);
    entries[2] = entry_create(tdups[2]);
    list_add(list, entries[2]);
    
    sprintf(t->tuple[0], "jkl");
    tdups[3] = tuple_dup(t);
    entries[3] = entry_create(tdups[3]);
    list_add(list, entries[3]);
    
    result = (list_remove(list, tdups[2]) == 0) &&
    (list_remove(list, tdups[1]) == 0) &&
    (list_get(list, tdups[0]) == entries[0]) &&
    (list_get(list, tdups[3]) == entries[3]) &&
    (list_size(list) == 2);
    
    tuple_destroy(t);
    list_destroy(list);
    
    printf("  Módulo list, teste remover meio: %s\n", result ? "passou" : "não passou");
    return result;
}

int main() {
    int score = 0;
    
    printf("\nIniciando o teste do módulo list\n");
    
    score += testListaVazia();
    score += testAddCabeca();
    score += testAddVarios();
    score += testRemoveCabeca();
    score += testRemoveCauda();
    score += testRemoveMeio();
    
    printf("Resultados do teste do módulo list: %d em 6\n\n", score);
    
    struct list_t * newList = list_create();
    list_destroy(newList);
    node_dup(NULL);
    
    entry_dup(NULL);
    
    return score;
}
