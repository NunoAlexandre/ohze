#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "message-private.h"
#include "message.h"

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
 Serialização e de-sserialização de CT_RESULT
 */
int testResult() {
    int result, size, res;
    short opcode, c_type;
    char *msg_str = NULL;
    struct message_t *msg = (struct message_t *) malloc(sizeof(struct message_t));
    
    msg->opcode = OC_OUT;
    msg->c_type = CT_RESULT;
    msg->content.result = 1;
    
    size = message_to_buffer(msg, &msg_str);
    
    opcode = htons(msg->opcode);
    c_type = htons(msg->c_type);
    res = htonl(msg->content.result);
    
    result = (memcmp(msg_str, &opcode, 2) == 0 &&
              memcmp(msg_str+2, &c_type, 2) == 0 &&
              memcmp(msg_str+4, &res, 4) == 0);
    
    free_message(msg);
    
    msg = buffer_to_message(msg_str, size);
    
    result = result && (msg->opcode == OC_OUT &&
                        msg->c_type == CT_RESULT &&
                        msg->content.result == 1);
    
    free(msg_str);
    free_message(msg);
    
    printf("  teste Result: %s\n", result ? "passou" : "não passou");
    return result;
}

/***********************************************************************
 Serialização e de-sserialização de CT_TUPLE
 */
int testTuple() {
    int result, size, tup_dim, el_size[3] = {htonl(5), htonl(2), htonl(4)};
    short opcode, c_type;
    char *msg_str = NULL;
    struct message_t *msg = (struct message_t *) malloc(sizeof(struct message_t));
    struct tuple_t *t;
    char *tdata[3] = {"AbCdE", "SD", "2014"};
    
    msg->opcode = OC_COPY;
    msg->c_type = CT_TUPLE;
    msg->content.tuple = tuple_create2(3, tdata);
    
    t = tuple_dup(msg->content.tuple);
    
    size = message_to_buffer(msg, &msg_str);
    
    opcode = htons(msg->opcode);
    c_type = htons(msg->c_type);
    tup_dim = htonl((int) 3);
    
    result = (memcmp(msg_str, &opcode, 2) == 0 &&
              memcmp(msg_str+2, &c_type, 2) == 0 &&
              memcmp(msg_str+4, &tup_dim, 4) == 0 &&
              memcmp(msg_str+8, &el_size[0], 4) == 0 &&
              memcmp(msg_str+12, tdata[0], 5) == 0 &&
              memcmp(msg_str+17, &el_size[1], 4) == 0 &&
              memcmp(msg_str+21, tdata[1], 2) == 0 &&
              memcmp(msg_str+23, &el_size[2], 4) == 0 &&
              memcmp(msg_str+27, tdata[2], 4) == 0);
    
    free_message(msg);
    msg = buffer_to_message(msg_str, size);
    
    result = result && (msg->opcode == OC_COPY &&
                        msg->c_type == CT_TUPLE &&
                        tuple_match(msg->content.tuple, t));
    
    free(msg_str);
    free_message(msg);
    tuple_destroy(t);
    
    printf("  teste Tuplo: %s\n", result ? "passou" : "não passou");
    return result;
}

/***********************************************************************
 Serialização e de-serialização de CT_ENTRY
 */
int testEntry() {
    int result, size, tup_dim, el_size[3] = {htonl(7), htonl(4), htonl(4)};
    long long ts;
    short opcode, c_type;
    char *msg_str = NULL;
    struct message_t *msg = (struct message_t *) malloc(sizeof(struct message_t));
    struct tuple_t *t, *t2;
    char *tdata[3] = {"AbCdEfG", "*SD*", "2014"};
    
    t = tuple_create2(3, tdata);
    t2 = tuple_dup(t);
    msg->opcode = OC_IN;
    msg->c_type = CT_ENTRY;
    msg->content.entry = entry_create(t);
    msg->content.entry->timestamp = 0x1F1F1F1F1F1F1F00;
    
    ts = swap_bytes_64(msg->content.entry->timestamp);
    
    size = message_to_buffer(msg, &msg_str);
    
    opcode = htons(msg->opcode);
    c_type = htons(msg->c_type);
    tup_dim = htonl((int) 3);
    
    result = (memcmp(msg_str, &opcode, 2) == 0 &&
              memcmp(msg_str+2, &c_type, 2) == 0 &&
              memcmp(msg_str+4, &ts, 8) == 0 &&
              memcmp(msg_str+12, &tup_dim, 4) == 0 &&
              memcmp(msg_str+16, &el_size[0], 4) == 0 &&
              memcmp(msg_str+20, tdata[0], 7) == 0 &&
              memcmp(msg_str+27, &el_size[1], 4) == 0 &&
              memcmp(msg_str+31, tdata[1], 4) == 0 &&
              memcmp(msg_str+35, &el_size[2], 4) == 0 &&
              memcmp(msg_str+39, tdata[2], 4) == 0);
    
    free_message(msg);
    
    msg = buffer_to_message(msg_str, size);
    
    result = result && (msg->opcode == OC_IN &&
                        msg->c_type == CT_ENTRY &&
                        tuple_match(msg->content.entry->value, t2));
    free(msg_str);
    
    free_message(msg);
    tuple_destroy(t2);
    
    printf("  teste Entry: %s\n", result ? "passou" : "não passou");
    return result;
}

/***********************************************************************
 De-serialização de lixo...
 */
int testInvalida() {
    int result;
    char *msg_rebenta = "\x00\x0A\x00\x64\x00\x00\x00\x05 Vai crashar esses programas todos. Têm de defender!";
    struct message_t *msg;
    
    msg = buffer_to_message(msg_rebenta, strlen(msg_rebenta)+1);
    
    result = (msg == NULL);
    
    free_message(msg);
    
    printf("  teste inválida: %s\n", result ? "passou" : "nãoo passou");
    return result;
}

/***********************************************************************
 main
 */
int main() {
    int score = 0;
    
    printf("\nIniciando o teste do módulo message\n");
    
    score += testResult();
    score += testTuple();
    score += testEntry();
    score += testInvalida();
    
    printf("Resultados do teste do módulo message: %d em 4\n\n",score);
    
    return score;
}
