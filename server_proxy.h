#ifndef _SERVER_PROXY_H_
#define _SERVER_PROXY_H_

#include "message.h"
#include "table_skel.h"
#include "network_server.h"


#define REQUESTS_BUCKET_SIZE 20
int number_of_proxies;

struct monitor_t {     // Um monitor pode ser implementado com um mutex e uma variável de condição
  pthread_mutex_t mut; 
  pthread_cond_t cvar;
};

/* Armazena dados relativos a uma THREAD de execução do switch.
*/
struct thread_data{
  struct request_t ** requests_bucket; // Apontador para a tabela de mensagens na THREAD principal
  int *bucket_has_requests;          // Apontador para variável de estado da tabela.
  struct monitor_t *monitor_bucket_has_requests; // Apontador oara o monitor do estado vazio da tabela
  pthread_mutex_t *bucket_access; // Apontador para o MUTEX de acesso à tabela
  char * server_address_and_port;
  short id;
};

/*
* A structure for a request
*/
struct request_t{
  short requestor_fd; 
  short flags; // 1 = ACK, 2 = NACK, ... Uso geral...
  struct message_t *request; // Mensagem recebida
  struct message_t *response; // Mensagem de resposta
  short deliveries;
  int acknowledged;  // Cada proxy, ao receber resposta decrementa esta
  int answered; // Já foi dada uma resposta ao cliente?
};



int monitor_init(struct monitor_t *mon); // Para inicializar o MUTEX e a variável de condição

/* Bloqueia até que o predicado seja verdadeiro.
*/
void monitor_wait(struct monitor_t *mon, int *predicate);

/* Sinaliza as threads que estiverem bloqueadas à espera que o predicado se verifique
*/
void monitor_signal(struct monitor_t *mon, int *predicate);




struct request_t * create_request_with(int socket_fd, struct message_t *request_msg,  struct message_t *response_msg, int flag,int proxies, int deliveries, int answered );

void request_free(struct request_t * request );


int get_number_of_proxies();

void set_number_of_proxies( int n );



void run_postman ( pthread_mutex_t * bucket_access, struct request_t ** bucket,
                  int * bucket_is_full, int *requests_counter, int * bucket_has_requests  );

/* Função que implementa um PROXY para um servidor e que será executada no âmbito
   de uma nova THREAD do processo SERVER_SWITCH.
   Recebe uma operação do SERVER_SWITCH, reencaminha-a para o servidor designado,
   recebe o resultado do servidor, e reencaminha-o para o SERVER_SWITCH.
*/
void *run_server_proxy(void *p);

#endif

