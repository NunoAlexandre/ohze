//
//  tuple-private.h
//  SD15-Project
//
//  Created by Grupo SD015 on 24/09/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#ifndef __SD15_Project__tuple_private__
#define __SD15_Project__tuple_private__

#include <stdio.h>
#include "tuple.h"

#define TUPLE_DIMENSION 3
#define TUPLE_ELEM_NULL "*"


char * tuple_element (struct tuple_t* tuple, int iElement ) ;
char * tuple_elem_str(struct tuple_t * tuple, int i);
char * tuple_key (struct tuple_t * tuple );
int tuple_size( struct tuple_t * tuple );
int tuple_size_bytes ( struct tuple_t* tuple);
int tuple_size_as_string (struct tuple_t* tuple) ;
char * tuple_to_string( struct tuple_t * tuple );
int tuple_serialize(struct tuple_t *tuple, char **buffer);
struct tuple_t *tuple_deserialize(char *buffer, int size);
struct tuple_t* create_tuple_from_input (const char *user_input);

void tuple_print ( struct tuple_t * tuple );

struct tuple_t ** tuple_create_array(int tuples_num);
void tuple_array_destroy(struct tuple_t ** tuples, int num );


#endif /* defined(__SD15_Project__tuple_private__) */
