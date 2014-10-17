//
//  message.c
//  SD15-Product
//
//  Created by Nuno Alexandre on 07/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#include <stdio.h>
#include "message.h"



/* Converte o conteúdo de uma message_t num char*, retornando o tamanho do
 * buffer alocado para a mensagem serializada como um array de
 * bytes, ou -1 em caso de erro.
 *
 * A mensagem serializada deve ter o seguinte formato:
 *
 * OPCODE		C_TYPE
 * [2 bytes]	[2 bytes]
 *
 * a partir daí, o formato difere para cada c_type:
 *
 * ct_type	dados
 * TUPLE	DIMENSION	ELEMENTSIZE	ELEMENTDATA	...
 *		[4 bytes]	 [4 bytes]	[ES bytes]
 * ENTRY	TIMESTAMP 	DIMENSION	ELEMENTSIZE ELEMENTDATA
 *		[8 bytes] 	[4 bytes]	 [4 bytes]	[ES bytes]	...
 * RESULT	RESULT
 *		[4 bytes]
 *
 */
int message_to_buffer(struct message_t *msg, char **msg_buf);

/* Transforma uma mensagem em buffer para uma struct message_t*
 */
struct message_t *buffer_to_message(char *msg_buf, int msg_size);

/* Liberta a memoria alocada na função buffer_to_message
 */
void free_message(struct message_t *msg);

