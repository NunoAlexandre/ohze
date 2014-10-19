#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "table-private.h"
#include "table.h"
#include "list-private.h"

#define GET_ALL 0
#define GET_ONE 1
#define KEEP_TUPLES KEEP_AT_ORIGIN
#define DEL_TUPLES DONT_KEEP_AT_ORIGIN

/************************************************************************/
/* Verifica se tup está de acordo com tup_template.                     */
/* Retorna 1 ou 0 (de acordo, em desacordo)                             */
/************************************************************************/
int tuple_match(struct tuple_t *tup, struct tuple_t *tup_template){
    int i;
    
    if (tup->tuple_dimension != tup_template->tuple_dimension)
        return 0;
    
    for(i = 0; i < tup->tuple_dimension; i++)
        if (tup_template->tuple[i] != NULL)
            if (strcmp(tup_template->tuple[i], tup->tuple[i]) != 0)
                return 0;
    return 1;
}
    /***********************************************************************
     Bem criada?
     */
    int testTabelaVazia(){
        struct table_t *table;
        
        table = table_create(5);
        
        int result = table != NULL && table_size(table) == 0;
        
        table_destroy(table);
        printf("  teste table vazia: %s\n", result ? "passou" : "nao passou");
        return result;
    }
    
    /***********************************************************************
     Multiplos outs
     */
    int testOut() {
        int result, i;
        struct table_t *table = table_create(10);
        
        char *tdata[3] = {"       ", "2014", "Fixe!"};
        struct tuple_t *t = tuple_create2(3, tdata);
        struct tuple_t *tdups[1024];
        
        struct list_t *res;
        
        for(i = 0; i < 1024; i++) {
            sprintf(t->tuple[0], "SD-%d", i);
            tdups[i] = tuple_dup(t);
            table_put(table, tdups[i]);
        }
        
        assert(table_size(table) == 1024);
        result = (table_size(table) == 1024);
        
        for(i=0; i<1024; i++) {
            res = table_get(table, tdups[i], KEEP_TUPLES, GET_ONE);
            assert(list_size(res) == 1);
            assert(tuple_match(res->head->entry->value, tdups[i]) == 1 || res->head->entry->value == tdups[i]);
            free(res->head);
            free(res);
        }
        
        assert(table_size(table) == 1024);
        result = result && (table_size(table) == 1024);
        
        tuple_destroy(t);
        
        table_destroy(table);
        
        printf("  teste out: %s\n", result ? "passou" : "não passou");
        return result;
    }
    
    /***********************************************************************
     Outs e gets com duplicados
     */
    int testDuplicados(){
        int result, i;
        struct table_t *table = table_create(10);
        char *tdata[3] = {"       ", "2014", "Fixe!"};
        struct tuple_t *t = tuple_create2(3, tdata);
        struct tuple_t *tdups[20];
        
        struct list_t *res;
        
        for(i = 0; i < 20; i++) {
            sprintf(t->tuple[0], "SD-%d", i);
            tdups[i] = tuple_dup(t);
            table_put(table, tdups[i]);
        }
        
        sprintf(t->tuple[0], "SD-%d", 10);
        table_put(table, t);
        
        assert(table_size(table) == 21);
        result = (table_size(table) == 21);
        
        for(i=0; i<20; i++) {
            res = table_get(table, tdups[i], DEL_TUPLES, GET_ALL);
            assert(list_size(res) == ((i == 10) ? 2 : 1));
            assert(tuple_match(res->head->entry->value, tdups[i]) == 1 || res->head->entry->value == tdups[i]);
            list_destroy(res);
        }
        assert(table_size(table) == 0);
        result = result && (table_size(table) == 0);
        
        table_destroy(table);
        
        printf("  teste duplicados: %s\n", result ? "passou" : "não passou");
        return result;
    }
    
    /***********************************************************************
     Com todos...
     */
    int testAll(){
        int result, i;
        struct table_t *table = table_create(10);
        char *tdata[3] = {"       ", "2014", "Fixe!"};
        struct tuple_t *t = tuple_create2(3, tdata);
        struct tuple_t *tdups[20];
        struct list_t *res;
        
        for(i = 0; i < 20; i++) {
            sprintf(t->tuple[0], "SD-%d", i);
            tdups[i] = tuple_dup(t);
            table_put(table, tdups[i]);
        }
        
        assert(table_size(table) == 20);
        result = (table_size(table) == 20);
        
        tuple_destroy(t);
        t=tuple_create(3);
        
        res = table_get(table, t, KEEP_TUPLES, GET_ALL);
        
        result = result && (list_size(res) == 20);
        
        result = result && (table_del(table, t, GET_ALL) == 0);
        
        assert(table_size(table) == 0);
        
        result = result && (table_size(table) == 0);
        
        table_destroy(table);
        tuple_destroy(t);
        list_destroy(res);
        
        printf("  teste All: %s\n", result ? "passou" : "não passou");
        return result;
    }
    
    int main() {
        int score = 0;
        
        printf("Iniciando o teste do módulo table\n");
        
        score += testTabelaVazia();
        score += testOut();
        score += testDuplicados();
        score += testAll();
        
        printf("Resultados do teste do módulo table: %d em 4\n",score);
        return score;
    }
