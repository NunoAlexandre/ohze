//
//  tuple-private.c
//  SD15-Project
//
//  Created by Nuno Alexandre on 24/09/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include "tuple-private.h"

char * getTupleKey (struct tuple_t * tuple ) {
    return tuple->tuple[0];
}
