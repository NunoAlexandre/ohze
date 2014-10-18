#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include "tuple.h"
//#include "entry.h"
#include "list.h"
#include "list-private.h"
#include "tuple-private.h"
#include "stdlib.h"
#include <string.h>

/***********************************************************************************/
/* "Meios" bem removidos?                                                          */
/***********************************************************************************/


int main() {
    puts("\n\n\n");
    
    char *tdata[3] = {"Banana", "2014", "Fisce!"};
    struct tuple_t *tuple = tuple_create2(3, tdata);
    
    struct entry_t * entry = entry_create2(tuple, 123);
    printf("Entry tem primeiro elemento com tamanho %lu \n", strlen(entry->value->tuple[0]));

    
    char **buffer = (char**) malloc (sizeof(char*) * 5);
    
    int bufferSize =  entry_serialize(entry, buffer);
    
    struct entry_t * entryAfter = entry_deserialize(buffer[0], bufferSize);
    
    puts("Depois serialize and deserialize: \n");
    printf("Original timestamp %lld and after is %lld\n", entry->timestamp, entryAfter->timestamp);

    printf("Original size %d and after size %d\n", tuple_size(tuple), tuple_size(entryAfter->value));
    printf("Original first elem %s and after first elem %s\n", tuple->tuple[0], entryAfter->value->tuple[0]);
    printf("Original snd elem %s and after snd elem %s\n", tuple->tuple[1], entryAfter->value->tuple[1]);
    printf("Original trd elem %s and after trd elem %s\n", tuple->tuple[2], entryAfter->value->tuple[2]);


    puts("\n\n\n");

       return 1;
}
