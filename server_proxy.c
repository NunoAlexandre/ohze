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

struct request_t * create_request_with(int socket_fd, struct message_t * request_msg,
                                       struct message_t * response_msg, int flag,int n_proxies, int deliveries, int answered )
{
    
    struct request_t * new_request = malloc(sizeof(struct request_t));
    // Alocar memória para uma mensagem local (entre thread principal e as outras)
    if ( new_request != NULL) {
        // Preparar mensagem local de acordo com o pedido do cliente
        new_request->requestor_fd = socket_fd;
        new_request->request = request_msg;
        new_request->response = response_msg;
        new_request->flags = flag;
        new_request->acknowledged = n_proxies;
        new_request->deliveries = deliveries;
        new_request->answered = answered;

    }
    
    return new_request;
}

void request_free(struct request_t * request ) {
    if ( request != NULL ) {
        free_message2(request->request,NO);
        free_message(request->response);
        free(request);
    }
}

int get_number_of_proxies() {
  return number_of_proxies;
}
void set_number_of_proxies( int number) {
  number_of_proxies = number;
}



void run_postman ( pthread_mutex_t * bucket_access, struct request_t ** bucket,
                  int * bucket_is_full, int *requests_counter, int * bucket_has_requests  ) {
    
    /* for better performance */
    if ( bucket_has_requests ) {
        int i;
        for ( i = 0; i < REQUESTS_BUCKET_SIZE; i++ ) {
            
            //resets the flag value for each request
            int failed_tasks = 0;
            
            /* locks the access to the bucket */
            pthread_mutex_lock(bucket_access);
            
            
            /** checks if the request i exists and got a response already **/
            if (bucket[i] != NULL && bucket[i]->response != NULL) {
                
                /* checks if there is the request i wasn't answered to the requestor yet */
                if (!bucket[i]->answered) {
                    
                    /** where all the response message will be stored **/
                    struct message_t ** response_messages = NULL;
                    
                    /** it will only invoke the request on itself if it went well on the other servers **/
                    if ( response_with_success(bucket[i]->request, bucket[i]->response) ) {
                        
                        int response_messages_num = invoke(bucket[i]->request, &response_messages);
                        // error case
                        failed_tasks+= response_messages_num <= 0 || response_messages == NULL;
                        
                        //sends the response to the client
                        int message_was_sent = server_send_response(bucket[i]->requestor_fd, response_messages_num, response_messages);
                        //error case
                        failed_tasks+= message_was_sent == FAILED;
                    }
                    else {
                        //declares that there was (at least) one failed that task once he
                        //response from the proxie was not a success response to the request.
                        failed_tasks = 1;
                    }
                    
                    /** IF some error happened, it will notify the client **/
                    if ( failed_tasks > 0 ) {
                        server_sends_error_msg(bucket[i]->requestor_fd);
                    }
                    
                    
                    /* if it went well it assumes if will also go well with all other proxies and
                     if went wrong we assume it also will with all other proxies,
                     and an error message was sent so the work for this request is done*/
                    bucket[i]->answered = YES;
                    
                }
                
                /* if the request was acknowledge by every proxy it can now be removed from the bucket */
                if (bucket[i]->acknowledged == 0) {
                    puts("\n\t--- request sent to all servers so will be removed from the bucket.");
                    request_free(bucket[i]);
                    bucket[i] = NULL;
                    *bucket_is_full = NO;
                    (*requests_counter)--;
                    *bucket_has_requests = *requests_counter > 0;
                }
            }
            /* by last, it unlocks the access to the bucket */
            pthread_mutex_unlock(bucket_access);
        }
    }

}




void * run_server_proxy ( void *p ) {

  /* stores the data of this proxy */
  struct thread_data *proxy = p;
   /* tries to connect to the server */
  struct server_t * server_to_contact = NULL;
  while ( (server_to_contact = network_connect(proxy->server_address_and_port)) == NULL )
      sleep(1);
   
    // flag to check fatal error
    int fatal_error = NO;
    //communication error tolerance
    int communication_failure_tolerance;
    // where each request_t to process will be stored
    struct request_t *request = NULL;
    //where each client request of each request_t will be stored
    struct message_t * client_request = NULL;
    // the bucket slot where to read a new request
    int index_to_read_request = 0;
    // where the server response will be stored
    struct message_t *server_response = NULL;
    
  
  while(!fatal_error) {
    
    //communication error tolerance
    communication_failure_tolerance = 3;
  
    /* waits until the bucket has requests to process */
    monitor_wait(proxy->monitor_bucket_has_requests, proxy->bucket_has_requests);

    /* accesses the bucket to get a request */
    pthread_mutex_lock(proxy->bucket_access);
     /* gets the requests from the bucket */
    request = proxy->requests_bucket[index_to_read_request];

    if ( (request != NULL) && (request->deliveries < get_number_of_proxies()) ) {
        
      request->deliveries++;
      client_request = request->request;

      /* unlocks the bucket */
      pthread_mutex_unlock(proxy->bucket_access);

     
      /* tries to get the server response while it fails to get it because the server socket is closed 
       and it doesnt manage to reconnect or it didnt cross the failure tolerance */
        //int error = 1;
        //while ( (communication_failure_tolerance-- > 0 ) && error ) {
        if ( server_to_contact == NULL || socket_is_open(server_to_contact->socketfd) )
            server_response = network_send_receive(server_to_contact, client_request);
        
                   // error = server_response == NULL;
            //if ( error ) {
               // while (communication_failure_tolerance-- > 0
                 //      && (server_to_contact = network_connect(proxy->server_address_and_port)) == NULL ) {
                  //  sleep(1);
                 //   printf("\t\t --- failed to connect to server. Will try %d more times\n", communication_failure_tolerance);
               // }
           // }
        //}
        
        
      /* it will commit the server_response to the bucket only if none error not socket caused occured */
      if ( server_response != NULL ) {
          printf("MESSAGE IS "); message_print(server_response); puts("");
        pthread_mutex_lock(proxy->bucket_access);
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

        if ( server_response == NULL || server_to_contact == NULL || socket_is_closed(server_to_contact->socketfd))
            server_to_contact = network_connect(proxy->server_address_and_port);

    }
    pthread_mutex_unlock(proxy->bucket_access); // Desbloquear a tabela
  }
    
   printf("\n\t--- error: proxie %d fell on a fatal error\n", proxy->id);
    
  return NULL;
}