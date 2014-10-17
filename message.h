#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "tuple.h"
#include "entry.h"

/* Define os possíveis opcodes da mensagem */
#define OC_OUT		10
#define OC_IN		20
#define OC_IN_ALL   	30
#define OC_COPY		40
#define OC_COPY_ALL	50
#define OC_SIZE		60

/* Define códigos para os possíveis conteúdos da mensagem */
#define CT_TUPLE  100
#define CT_ENTRY  200
#define CT_RESULT 300

/* Estrutura que representa uma mensagem genérica a ser transmitida.
 * Esta mensagem pode ter vários tipos de conteúdos.
 */
struct message_t {
	short opcode; /* código da operação na mensagem */
	short c_type; /* tipo do conteúdo da mensagem */
	union content_u {
		struct tuple_t *tuple;
		struct entry_t *entry;
		int result;
	} content; /* conteúdo da mensagem */
};

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
#endif
