//
//  tuple-private.h
//  SD15-Project
//
//  Created by Nuno Alexandre on 24/09/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#ifndef __SD15_Project__tuple_private__
#define __SD15_Project__tuple_private__

#include <stdio.h>
#include "tuple.h"

char * tuple_element (struct tuple_t* tuple, int iElement ) ;
char * tuple_key (struct tuple_t * tuple );
int tuple_size( struct tuple_t * tuple );
int tuple_size_bytes ( struct tuple_t* tuple);

int tuple_serialize(struct tuple_t *tuple, char **buffer);
struct tuple_t *tuple_deserialize(char *buffer, int size);

#endif /* defined(__SD15_Project__tuple_private__) */
