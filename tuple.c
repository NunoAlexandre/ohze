//
//  tuple.c
//  SD_2014
//
//  Created by Bruno Mata on 22/09/14.
//
//

#include "tuple.h"
#include <stdlib.h>


/* Função que destrói um bloco de dados e liberta toda a memoria.
 */
void tuple_destroy(struct tuple_t *tuple){
    free(&(tuple->tuple_dimension));
    free(tuple->tuple);
    free(tuple);
}