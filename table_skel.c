#include "table_skel.h"
#include "table_skel-private.h"
#include "table.h"
#include "table-private.h"
#include "message.h"
#include "message-private.h"
#include "general_utils.h"
#include <stdlib.h>
#include "table.h"
#include "server_log.h"

/*
 * The table where everything will happen
 */
struct table_t * table = NULL;

long long latest_put_timestamp;


void table_skel_init_log( char * filepath ) {
    server_log_init(filepath);
}

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta funcao antes de usar a
 * funcao invoke(). O parametro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists) {
	//case called more than once
 	if ( table != NULL ) {
 		perror("table_skel_init > table already initialized");
 		return SUCCEEDED;
 	}
	//creates the server table
 	table = table_create(n_lists);
    
    
	//returns success value
 	return table != NULL ? SUCCEEDED : FAILED;
}

int table_skel_init_with(int n_lists, int response_mode, int checklog, int logging, char * address_and_port ) {
    if ( table_skel_init(n_lists) == FAILED )
        return FAILED;
    
    if ( logging )
        table_skel_init_log(address_and_port);

    if ( checklog ) {
        RESPONSE_MODE = MUTE_RESPONSE_MODE;
        logging_on = NO;
        server_log_invoke_over_table(table);
    }
    logging_on = logging;
    RESPONSE_MODE = response_mode;
    latest_put_timestamp = 0;
    
    return SUCCEEDED;
}

/* Libertar toda a memória e recursos alocados pela função anterior.
 */
int table_skel_destroy() {
	// destroyes the table
 	table_destroy(table);
    
 	return SUCCEEDED;
}

int init_response_with_message(struct message_t *** msg_set_out, int set_size, struct message_t * first_message) {
    if ( RESPONSE_MODE !=  MUTE_RESPONSE_MODE ) {
        //the array msg_set_out points to is now a set with one pointer to a message
        *msg_set_out = message_create_set(set_size);
        //error case
        if ( *msg_set_out == NULL) return FAILED;
    
        //so the first elem of the array is the message with the table size
        (*msg_set_out)[0] = first_message;
        //error case
        if ((*msg_set_out)[0] == NULL ) return FAILED;
    }
    else
        set_size = 0;

 	return set_size;
}

int table_skel_error(struct message_t *** msg_set_out ) {
 	return init_response_with_message(msg_set_out, 1, message_of_error());
}

int table_skel_size (struct message_t * msg_in, struct message_t *** msg_set_out ) {
 	int tablesize = table_size(table);
	return init_response_with_message(msg_set_out, 1, message_create_with(msg_in->opcode+1, CT_RESULT, &tablesize));
}
int table_skel_put (struct message_t * msg_in, struct message_t *** msg_set_out ) {
    
 	//puts the new tuple or entry
 	int successValue = msg_in->c_type == CT_TUPLE ?
    table_put(table, msg_in->content.tuple) : table_put_entry(table, msg_in->content.entry);
    
    /* updates the latest_put_timestamp */
    if ( msg_in->c_type == CT_ENTRY )
        latest_put_timestamp = msg_in->content.entry->timestamp;
    
	//so the first elem of the array is the message with the success value
 	return init_response_with_message(msg_set_out, 1, message_create_with(msg_in->opcode+1, CT_RESULT, &successValue));
}

int action_on_get_tuples (struct message_t * operation ) {
    if ( RESPONSE_MODE == MUTE_RESPONSE_MODE )
        return DONT_KEEP_AT_ORIGIN;
    else if ( message_opcode_taker(operation) )
        return DONT_KEEP_AT_ORIGIN;

    return KEEP_AT_ORIGIN;
}

int table_skel_get (struct message_t * msg_in, struct message_t *** msg_set_out ) {
    
	//have to know if should keep the matching nodes or not
 	int whatToDoWithTheTuples = action_on_get_tuples(msg_in);
	//if is IN or COPY value is 1 (just one), 0 otherwise meaning "all"
 	int one_or_all =  msg_in->opcode == OC_IN ||  msg_in->opcode == OC_COPY;
    
	//pointer to the tuple template
 	struct tuple_t * tup_template = msg_in->content.tuple;
	//where the matching tuples will be stored
 	struct tuple_t ** matching_tuples = NULL;
 	//gets the matching tuples
 	int matching_tuples_num = table_get_array(table, tup_template, whatToDoWithTheTuples, one_or_all, &matching_tuples);
    
 	int n_msgs = tuples_to_message_array(msg_in, &matching_tuples, matching_tuples_num, msg_set_out);
    
 	return n_msgs;
}
void table_skel_print() {
 	table_print(table);
}

int tuples_to_message_array( struct message_t * msg_in, struct tuple_t *** matching_tuples, int ntuples, struct message_t *** msg_set_out) {
    
    
    //the opcode of each message will be this
 	int msgs_opcode = msg_in->opcode + 1;
    
    //the number of responses depends on the response mode
    int msg_set_size = RESPONSE_MODE == SERVER_RESPONSE_MODE && message_opcode_taker(msg_in) ? 1 : 1 + ntuples;
    /* creates a set with the proper number of response messages */
    int n_messages = init_response_with_message(msg_set_out, msg_set_size, message_create_with(msgs_opcode, CT_RESULT, &ntuples));
    
    if ( n_messages == FAILED)
    	return FAILED;
    
    /* it will only run if response mode is switch mode */
 	int i = 1;
 	int sent_successfully = YES;
 	while ( i <= (n_messages-1) && sent_successfully ) {
 		//saves the message with the tuple...
 		(*msg_set_out)[i] = message_create_with(msgs_opcode, CT_TUPLE, (*matching_tuples)[i-1]);
 		
 		//if error stops
 		if ((*msg_set_out)[i] == NULL )
			sent_successfully = NO;
        
		//moves forward
 		i++;
 	}
    //number of message is the first message saying number of nodes followed by each tuple message
 	return  sent_successfully ? n_messages : FAILED;
}




/* Executa uma operação (indicada pelo opcode na msg_in) e retorna o(s)
 * resultado(s) num array de mensagens (struct message_t **msg_set_out).
 * Retorna o número de mensagens presentes no array msg_set_out ou -1
 * (erro, por exemplo, tabela não inicializada).
 */
int invoke(struct message_t *msg_in, struct message_t ***msg_set_out) {
 	if ( table == NULL ) {
        printf(" INVOKE! > table == NULL");
 		return FAILED;
    }
    
 	if ( ! message_valid_opcode(msg_in))
		return FAILED;
    
	//by default the number of messages is FAILED
	int number_of_msgs = FAILED;
	//then is set to a value depending on the
	if ( message_opcode_setter(msg_in)) {
		number_of_msgs = table_skel_put(msg_in,msg_set_out);
	}
	else if ( message_opcode_getter(msg_in)) {
		number_of_msgs = table_skel_get(msg_in, msg_set_out);
	}
	else if ( message_opcode_size (msg_in) ) {
		number_of_msgs = table_skel_size(msg_in, msg_set_out);
	}
    
    if ( logging_on && (msg_in->opcode == OC_OUT || msg_in->opcode == OC_IN || msg_in->opcode == OC_IN_ALL) ) {
            server_log_message(msg_in);
    }
        
	return number_of_msgs;
}








