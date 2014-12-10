#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "tuple.h"
#include "entry.h"

/* Define os possíveis opcodes da mensagem */
#define OC_OUT		10 //insere um tuplo
#define OC_IN		20 //lê e remove 1 tuplo
#define OC_IN_ALL   30 //lê e remove TODOS
#define OC_COPY		40 //lê e copia 1 tuplo
#define OC_COPY_ALL	50 //lê e copia TODOS
#define OC_SIZE		60 //
#define OC_REPORT   70 //mensagem com informação acerca do estado do switch

/* Define códigos para os possíveis conteúdos da mensagem */
#define CT_TUPLE    100 //mensagem de tuplo
#define CT_ENTRY    200 //mensagem de entry
#define CT_RESULT   300 //mensagem com resultado da operação
#define CT_SFAILURE 400 //mensagem com informação de endereço_ip:porta do switch que falhou
#define CT_SRUNNING 500 //mensagem com informação de endereço_ip:porta do novo switch
#define CT_INVCMD 600 //mensagem para informar comando invalido
/*
 * Estrutura que representa uma mensagem genérica a ser transmitida.
 * Esta mensagem pode ter vários tipos de conteúdos.
 */
struct message_t {
	short opcode; /* código da operação na mensagem */
	short c_type; /* tipo do conteúdo da mensagem */
	union content_u {
		struct tuple_t *tuple;
		struct entry_t *entry;
		int result;
        char *token;
	} content; /* conteúdo da mensagem */
};

/*
 * Converte o conteúdo de uma message_t num char*, retornando o tamanho do
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
 * TUPLE	DIMENSION   ELEMENTSIZE ELEMENTDATA	...
 *          [4 bytes]   [4 bytes]   [ES bytes]
 * ENTRY	TIMESTAMP   DIMENSION   ELEMENTSIZE ELEMENTDATA
 *          [8 bytes]   [4 bytes]   [4 bytes]	[ES bytes]	...
 * RESULT	RESULT
 *          [4 bytes]
 *
 * REPORT   REPORTSIZE  REPORTDATA
 *          [4 bytes]   [RD bytes]
 */
int message_to_buffer(struct message_t *msg, char **msg_buf);

/*
 * Transforma uma mensagem em buffer para uma struct message_t*
 */
struct message_t *buffer_to_message(char *msg_buf, int msg_size);

/*
 * Liberta a memoria alocada na função buffer_to_message
 */
void free_message(struct message_t *msg);

#endif
