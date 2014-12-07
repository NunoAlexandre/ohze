#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "server_proxy.h"
#include "network_cliente.h"
#include "message.h"

int monitor_init(struct monitor_t *mon){
  pthread_mutex_init(&mon->mut, NULL);
  return pthread_cond_init(&mon->cvar, NULL);
}


void monitor_wait(struct monitor_t *mon, int *predicate) {

  pthread_mutex_lock(&mon->mut);

  while (!(*predicate))
    pthread_cond_wait(&mon->cvar, &mon->mut);

  pthread_mutex_unlock(&mon->mut);

}


void monitor_signal(struct monitor_t *mon, int *predicate) {

  pthread_mutex_lock(&mon->mut);

  if (*predicate)
    pthread_cond_broadcast(&mon->cvar);

  pthread_mutex_unlock(&mon->mut);

}


void * run_server_proxy ( void *p ) {

  int fatal_error = NO;

  /* where each request to process will be stored */
  struct request_t *request = NULL;
  /* the slot where to read a new request */
  int index_to_read_request = 0; 
  /* stores the data of this proxy */
  struct thread_data *proxy = p;

  /** this proxy connects to its own remote table/server **/
  struct server_t * server_to_contact = NULL;
  if ( (server_to_contact = network_connect(proxy->server_address_and_port)) == NULL ) {
    puts("--- fatal error: proxy failed to connect to its server");
    fatal_error = YES;
  }
  
  while(!fatal_error) {
    
    // Bloquear enquanto a tabela estiver vazia
    monitor_wait(proxy->monitor_bucket_has_requests, proxy->bucket_has_requests);

    // Aceder à tabela e ler uma mensagem
    pthread_mutex_lock(proxy->bucket_access);

    if ( (request = proxy->requests_bucket[index_to_read_request]) != NULL) {

      index_to_read_request = index_to_read_request+1 % REQUESTS_BUCKET_SIZE;

      pthread_mutex_unlock(proxy->bucket_access); // Desbloquear a tabela

      // Seria a altura de fazer send_receive para o tabel_server,
      // de gerir falhas de ligação, etc...
      struct message_t *server_response = network_send_receive(server_to_contact, request->request);


      sleep(2); 

      // Acabou o send_receive!
      // Colocar a resposta na tabela

      pthread_mutex_lock(proxy->bucket_access); // Acesso à tabela

      if (request->response == NULL) { // Alguma thread já colocou a resposta?
        request->response = server_response; 
        request->flags = 1;  // 1: ACK, 2: NACK
      }

      request->acknowledged--;                   // Contabilizar a leitura e processamento desta thread.
    }

    pthread_mutex_unlock(proxy->bucket_access); // Desbloquear a tabela
    
  }
  return NULL;
}

