//
//  entry-private.c
//  SD15-Project
//
//  Created by Nuno Alexandre on 24/09/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include "entry-private.h"
#include "tuple-private.h"

char * getEntryKey (struct entry_t * entry) {
     return getTupleKey(entry->value);
}
