#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include "tuple.h"
//#include "entry.h"
#include "list.h"
#include "list-private.h"
#include "tuple-private.h"



/***********************************************************************************/
/* "Meios" bem removidos?                                                          */
/***********************************************************************************/


int main() {
    puts("\n\n\n");
    
    char *tdata[3] = {"Banana", "2014", "Fisce!"};
    struct tuple_t *tuple = tuple_create2(3, tdata);
    
    char **buffer;
    
    int bufferSize =  tuple_serialize(tuple, buffer);
    
    printf("Buffer serialized is %s\n",*buffer);
    struct tuple_t * tupleAfter = tuple_deserialize(buffer[0], bufferSize);
    
    puts("Depois serialize and deserialize: \n");
    
    printf("Original size %d and after size %d\n", tuple_size(tuple), tuple_size(tupleAfter));
    printf("Original first elem %s and after first elem %s\n", tuple->tuple[0], tupleAfter->tuple[0]);
    printf("Original snd elem %s and after snd elem %s\n", tuple->tuple[1], tupleAfter->tuple[1]);
    printf("Original trd elem %s and after trd elem %s\n", tuple->tuple[2], tupleAfter->tuple[2]);


    puts("\n\n\n");

       return 1;
}
