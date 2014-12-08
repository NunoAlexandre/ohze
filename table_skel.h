#ifndef _TABLE_SKEL_H
#define _TABLE_SKEL_H

#include "message.h"

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta funcao antes de usar a
 * funcao invoke(). O parametro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
 int table_skel_init(int n_lists);

/* Libertar toda a memória e recursos alocados pela função anterior.
 */
 int table_skel_destroy();

/* Executa uma operação (indicada pelo opcode na msg_in) e retorna o(s)
 * resultado(s) num array de mensagens (struct message_t **msg_set_out).
 * Retorna o número de mensagens presentes no array msg_set_out ou -1 
 * (erro, por exemplo, tabela não inicializada).
 */
 int invoke(struct message_t * msg_in, struct message_t ***msg_set_out);

#endif
