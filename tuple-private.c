//
//  tuple-private.c
//  SD15-Project
//
//  Created by Nuno Alexandre on 24/09/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include "tuple-private.h"

char * tuple_key (struct tuple_t * tuple ) {
    return tuple->tuple[0];
}

int tuple_size( struct tuple_t * tuple ) {
    return tuple->tuple_dimension;
}

char * tuple_element ( struct tuple_t * tuple, int iElement ) {
    return tuple->tuple[iElement];
}
