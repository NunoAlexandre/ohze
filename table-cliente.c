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
 * tests input number of arguments
 */
int test_input(int argc){
    if (argc != 2){
        /* MESSAGE INUPUT ERROR */
        printf("Uso: ./table_client <servidor>:<porto>\n");
        printf("Exemplo de uso 1: ./table_client 10.10.10.10:805\n");
        printf("Exemplo de uso 2: ./table_client wwww.example.com:805\n");
        
        return TASK_FAILED;
    }
    
    return 0;
}




/*
 *  Message if port number is invalid
 */
void invalid_port_number_message () {
    puts("####### SD15-SERVER ##############");
    puts("Sorry, your input was not valid.");
    puts("You must provide a valid number to be the server port.");
    puts("NOTE: Port invalid if (portNumber >=1 && portNumber<=1023) OR (portNumber >=49152 && portNumber<=65535)");
    puts("####### SD15-SERVER ##############");
}




int main(int argc , char *argv[]) {
    
    /* 0. SIGPIPE Handling */
    struct sigaction s;
    //what must do with a signal - ignore
    s.sa_handler = SIG_IGN;
    //set what to do when gets the SIGPIPE
    sigaction(SIGPIPE, &s, NULL);
    
    /* 1. Testar input de utilizador*/
    puts("1. Testar input de utilizador\n");
    test_input (argc);
    
    struct server_t *server_to_conect = network_connect(argv[1]);
    puts("A ligar a servidor...");
    
    /* 6.1 Verificação da ligação ao SERVIDOR */
    if (server_to_conect == NULL) {
        perror("ERRO AO LIGAR AO SERVIDOR\n");
        
        return TASK_FAILED;
    }
    
    /* 7. Tratamento do comando do utilizador */
    puts ("Introduzir comando:");
    
    int keepOn = YES;
    char input [MAX_MSG]; //vai guardar o comando do utilizador
    
    /* 7.1 Ciclo que trata do user input */
    while ( keepOn ) {

        //reads user command
        if (fgets (input, MAX_MSG-1, stdin) == NULL){
            return TASK_FAILED;
        }
        
        /* 7.5 Criação da mensagem a enviar */
        struct message_t * request_msg = command_to_message(input);
        
        if ( request_msg == NULL ) {
            keepOn = NO;
        }
        
        if ( keepOn ) {
            /* 7.6 Envio da mensagem */
            puts("before network_send_receive");
            struct message_t * received_msg = network_send_receive(server_to_conect, request_msg);
            
            /* 7.7 Verificação da mensagem de resposta */
            if (response_with_success(request_msg, received_msg) == NO ){
                perror(" ******** Response without success ********* \n");
            }
            
            printf("## MENSAGEM DE RESPOSTA:\n");
            printf("RESULT IS %d\n", received_msg->content.result);
            
            int tuplesToRead =request_msg->opcode == OC_IN ? received_msg->content.result : 0;
            
            printf("TUPLES TO READ IS %d\n", tuplesToRead);
            while ( tuplesToRead > 0 ) {
                printf("Reading a tuple...");
                receive_message(server_to_conect->socketfd);
                tuplesToRead--;
            }
        }
        
    }
    
    puts("A terminar sessão...");
    return TASK_SUCCEEDED;
}