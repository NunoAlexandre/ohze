#ifndef SD15_Product_table_skel_private_h
#define SD15_Product_table_skel_private_h

#include "list.h"

#define SERVER_RESPONSE_MODE 0
#define SWITCH_RESPONSE_MODE 1
#define MUTE_RESPONSE_MODE 2

/** Module Properties */
// defines the way that the table operates or responds to operations **/
int RESPONSE_MODE;
// sets if it must log operations or not
int logging_on;
// the timestam of the last timestamp
long long latest_put_timestamp;
// number of operations made
int n_write_operations;


int table_skel_write_operations();

void table_skel_update_neighboor (int neighbor_fd, struct message_t * msg_in );

long long table_skel_latest_put_timestamp();

void table_skel_set_response_mode(int mode );
int table_skel_get_response_mode() ;
long long table_skel_latest_put_timestamp();

void table_skel_init_log( char * filepath );

int table_skel_init_with(int n_lists, int response_mode, int checklog, int logging, char * address_and_port );
/*
* Initiliazes the msg_set_out array with set_size and sets its first_message as its first message.
*/
int init_response_with_message(struct message_t *** msg_set_out, int set_size, struct message_t * first_message);
/*
* Sets the msg_set_out to an error message.
*/
int table_skel_error(struct message_t *** msg_set_out );
/*
* Sets msg_set_out with a message with the table size.
*/
int table_skel_size (struct message_t * msg_in, struct message_t *** msg_set_out );
/*
* Sets msg_set_out with a first message with the number of 
* messages with tuples that it will be followed by.
*/
int table_skel_get (struct message_t * msg_in, struct message_t *** msg_set_out );
/*
* Sets msg_set_out with a message with the task success 
* of putting the tuple of msg_in into the table.
*/
int table_skel_put (struct message_t * msg_in, struct message_t *** msg_set_out );
/*
* Sets msg_set_out with a first message with the number of 
* messages with tuples that it will be followed by.
*/
int tuples_to_message_array( struct message_t * msg_in, struct tuple_t *** matching_tuples, int ntuples, struct message_t *** msg_set_out);

int list_to_message_array( struct message_t * msg_in, struct list_t * list, int gotBy, struct message_t *** msg_set_out);
/*
* Prints the table
*/
void table_skel_print();

#endif
