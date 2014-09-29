//
//  entry-private.h
//  SD15-Project
//
//  Created by Nuno Alexandre on 24/09/14.
//  Copyright (c) 2014 Grupo SD015. All rights reserved.
//

#ifndef __SD15_Project__entry_private__
#define __SD15_Project__entry_private__

#include <stdio.h>
#include "entry.h"

/*
 * Returns the key of a given entry.
 */
char * entry_key (struct entry_t * entry);

/*
 * Returns the value (tuple) of a given entry.
 */
struct tuple_t * entry_value ( struct entry_t * entry );


#endif /* defined(__SD15_Project__entry_private__) */
