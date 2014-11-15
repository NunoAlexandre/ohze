#ifndef SD15_Product_table_skel_private_h
#define SD15_Product_table_skel_private_h

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

/*
* Prints the table
*/
void table_skel_print();

#endif
