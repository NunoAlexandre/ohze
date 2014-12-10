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
#include "network_utils.h"

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

int get_number_of_proxies() {
  return number_of_proxies;
}
void set_number_of_proxies( int number) {
  number_of_proxies = number;
}


void * run_server_proxy ( void *p ) {

  int fatal_error = NO;

  /* where each request to process will be stored */
  struct request_t *request = NULL;
  /* the slot where to read a new request */
  int index_to_read_request = 0; 
  /* stores the data of this proxy */
  struct thread_data *proxy = p;

  /** announcing its work */
  printf("\t--- proxy %d on work \n", proxy->id );
  /** this proxy connects to its own remote table/server **/

  struct server_t * server_to_contact = NULL;
  while ( (server_to_contact = network_connect(proxy->server_address_and_port)) == NULL ) {
    //once it didnt manage to connect its not available
    //proxy->is_available = NO;
    //and sleeps 1 second before trying again
    sleep(1);
  }

  /* by getting into here the thread is connected with its server so its available*/
  //proxy->is_available = YES;
  
  while(!fatal_error) {
    
    // Bloquear enquanto a tabela estiver vazia
    monitor_wait(proxy->monitor_bucket_has_requests, proxy->bucket_has_requests);

    /* accesses the bucket to get a request */
    pthread_mutex_lock(proxy->bucket_access);

    request = proxy->requests_bucket[index_to_read_request];

    if ( (request != NULL) && (request->deliveries < get_number_of_proxies()) ) {

      request->deliveries++;

      pthread_mutex_unlock(proxy->bucket_access); // Desbloquear a tabela

      /* where the server response will be stored */
      struct message_t *server_response = NULL;

      /** while it fails to send or receive from the server because 
      its socket got closed it keeps trying with 1 second between **/
      struct message_t * theRequest = request->request;
      while ( ((server_response = network_send_receive(server_to_contact, theRequest)) == NULL) 
        && socket_is_closed(server_to_contact->socketfd) ) 
      {
        //while retrying it the thread is not available to get new requests
        //proxy->is_available = NO;
        //and sleeps 1 second before trying again
        sleep(1);
      }
          
      /* by getting into here means that there is estabelished connection with the server so thread is available */
     // proxy->is_available = YES;

      /** it will commit the server_response to the bucket only if none error not socket caused occured **/
      if ( server_response != NULL ) {
        pthread_mutex_lock(proxy->bucket_access); // Acesso à tabela

        /* if none proxy has given an answer so far, it will store it on the bucket */
        if ( request->response == NULL ) { 
          request->response = server_response; 
          request->flags = 1;  // 1: ACK, 2: NACK
        }
      }
      /* threads commits that acknowledged this request */
      request->acknowledged--;
      /* goes a place forward in a cicular way */
      index_to_read_request = (index_to_read_request+1) % REQUESTS_BUCKET_SIZE;

    }

    pthread_mutex_unlock(proxy->bucket_access); // Desbloquear a tabela
  }
  return NULL;
}