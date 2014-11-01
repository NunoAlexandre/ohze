#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> //For errno - the error number
#include <netdb.h> //hostent
#include <signal.h>

#include "inet.h"
#include "table.h"
#include "network_cliente.h"
#include "general_utils.h"
#include "general_utils.h"
#include "network_utils.h"


/*
 *  Message if port number is invalid
 */
void invalid_client_input () {
    puts("\n\n####### SD15-CLIENT ##############");
    puts("Sorry, your input was not valid.");
    puts("Uso: ./SD15_CLIENT <servidor>:<porto>");
    puts("Exemplo de uso 1: ./SD15_CLIENT 10.10.10.10:1250");
    puts("Exemplo de uso 2: ./SD15_CLIENT wwww.example.com:1250");
    puts("####### SD15-CLIENT ##############\n\n");
}

void invalid_command () {
    puts("\n\n####### SD15-CLIENT ##############");
    puts("Sorry, your command was not valid.");
    puts("IN | IN_ALL | COPY | COPY_ALL | OUT \"elem1\" \"elem2\" \"elem3\"");
    puts("SIZE | QUIT");
    puts("####### SD15-CLIENT ##############\n\n");
}

/*
 * tests input number of arguments
 */
int test_input(int argc){
    if (argc != 2){
        invalid_client_input();
        return TASK_FAILED;
    }
    return TASK_SUCCEEDED;
}

int opcode_is_getter (int opcode ) {
    return opcode == OC_IN || opcode == OC_IN_ALL || opcode == OC_COPY || opcode == OC_COPY_ALL;
}

/*
 * Assume que a msg_response é uma mensagem de sucesso para com a msg_request.
 */
int client_decision_to_take (struct message_t * msg_request, struct message_t * msg_response ) {
    if ( opcode_is_getter(msg_request->opcode) && msg_response->content.result > 0 ) {
        return CLIENT_RECEIVE_TUPLES;
    }
    return CLIENT_PROCEED;
}

int main(int argc , char *argv[]) {
        
    /* 0. SIGPIPE Handling */
    struct sigaction s;
    //what must do with a signal - ignore
    s.sa_handler = SIG_IGN;
    //set what to do when gets the SIGPIPE
    sigaction(SIGPIPE, &s, NULL);
    
    /* 1. Testar input de utilizador*/
    if ( test_input (argc) == TASK_FAILED ) return TASK_FAILED;
    
    
    //to keep the active
    int keepGoing = YES;
    //para guardar o comando do utilizador
    char input [MAX_MSG];
    //the server to connect to
    struct server_t *server_to_conect;
    char * server_address_and_port = strdup(argv[1]);
    
    /* Lê comando do utilizador e faz pedidos ao servidor */
    while ( keepGoing ) {
        
        puts("\n---- Introduzir comando ----");
        //reads user command
        if (fgets (input, MAX_MSG-1, stdin) == NULL) {
            free(server_address_and_port);
            return TASK_FAILED;
        }
        
        // Cria mensagem a partir do input
        struct message_t * request_msg = command_to_message(input);
        
        //Processa a mensagem de pedido adequadamente...
        if ( request_msg == NULL ) {
            puts("ERROR: Erro ao processar mensagem de pedido ao servidor");
        }
        else if ( request_msg->opcode == OC_QUIT ) {
            keepGoing = NO;
        }
        else if ( request_msg->opcode == OC_DOESNT_EXIST ) {
            invalid_command();
        }
        else {
            /*******    O comando foi correcto e vai proceder à comunicacao com o servidor *******/
            //for each command it connects with the server...
            server_to_conect = network_connect(server_address_and_port);
            if (server_to_conect == NULL) {
                invalid_client_input();
                free(server_address_and_port);
                return TASK_FAILED;
            }
            
            //envia o pedido e guarda a mensagem de resposts
            struct message_t * received_msg = network_send_receive(server_to_conect, request_msg);
            
            // Verificação da mensagem de resposta
            if ( response_with_success(request_msg, received_msg)  ) {
                //checks what has to do now...
                if ( client_decision_to_take(request_msg, received_msg) == CLIENT_RECEIVE_TUPLES ) {
                    printf("--- has %d tuples to get from the server.\n", received_msg->content.result);
                    int tuplesToRead =  received_msg->content.result;
                    
                    while ( tuplesToRead > 0 ) {
                        receive_message(server_to_conect->socketfd);
                        tuplesToRead--;
                    }
                } //acaba de ler os tuplos enviados pelo servidor
            }
            free_message(received_msg);
        }
        //vai voltar a fazer um pedido...
        free_message(request_msg);
    }
    
    //tells server to close
//    network_close(server_to_conect);
    puts("A terminar sessão...");
    free(server_address_and_port);
    //termina processo com sinal de successo
    return TASK_SUCCEEDED;
}