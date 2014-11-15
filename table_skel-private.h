#ifndef SD15_Product_table_skel_private_h
#define SD15_Product_table_skel_private_h

int table_skel_error(struct message_t *** msg_set_out );
int table_skel_size (struct message_t * msg_in, struct message_t *** msg_set_out );
int table_skel_get (struct message_t * msg_in, struct message_t *** msg_set_out );
int table_skel_put (struct message_t * msg_in, struct message_t *** msg_set_out );
int tuples_to_message_array( struct message_t * msg_in, struct tuple_t *** matching_tuples, int ntuples, struct message_t *** msg_set_out);
int resolve_response (struct message_t *msg_in, struct message_t ***msg_set_out);
void table_skel_print();

#endif
