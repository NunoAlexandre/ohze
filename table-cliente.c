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

/*
 *  Find opcode form user input
 */
int find_opcode(char *input_dup){
    
    char *user_task = strdup(strtok(input_dup," "));
    
    char *out = "out";
    char *in = "in";
    char *in_all = "in_all";
    char *copy = "copy";
    char *copy_all = "copy_all";
    char *size = "size\n";
    char *quit = "quit\n";
    
    printf("OPCODE -> %s\n", user_task);
    
    if (strcasecmp(user_task, out) == 0) {
        printf("OPCODE -> OC_OUT\n");
        printf ("Instrução: %s\n", user_task);
        free(user_task);
        return OC_OUT;
    }
    
    if (strcasecmp(user_task, in) == 0) {
        printf("OPCODE -> OC_IN\n");
        printf ("Instrução: %s\n", user_task);
        free(user_task);
        return OC_IN;
    }
    
    if (strcasecmp(user_task, in_all) == 0) {
        printf("OPCODE -> OC_IN_ALL\n");
        printf ("Instrução: %s\n", user_task);
        free(user_task);
        return OC_IN_ALL;
    }
    
    if (strcasecmp(user_task, copy) == 0) {
        printf("OPCODE -> OC_COPY\n");
        printf ("Instrução: %s\n", user_task);
        free(user_task);
        return OC_COPY;
    }
    
    
    if (strcasecmp(user_task, copy_all) == 0) {
        printf("OPCODE -> OC_COPY_ALL\n");
        printf ("Instrução: %s\n", user_task);
        free(user_task);
        return OC_COPY_ALL;
    }
    
    
    if (strcasecmp(user_task, size) == 0) {
        printf("OPCODE -> OC_SIZE\n");
        printf ("Instrução: %s\n", user_task);
        free(user_task);
        return OC_SIZE;
    }
    
    if (strcasecmp(user_task, quit) == 0) {
        printf("OPCODE -> QUIT\n");
        printf ("Instrução: %s\n", user_task);
        free(user_task);
        return TASK_SUCCEEDED;
    }
    
    
    else {
        puts("COMANDO DE OPERAÇÂO INVÁLIDo!\n");
    }
    
    free(user_task);
    return TASK_FAILED;
}

/*
 *  Assigns CTCODE according with OPCODE
 */
int assign_ctcode (int opcode){
    
    int ctcode = TASK_FAILED;
    
    switch (opcode) {
            
            /* all operations that envolves finding elements from table */
        case OC_IN:
        case OC_COPY:
        case OC_IN_ALL:
        case OC_COPY_ALL:
        case OC_OUT:
        {
            ctcode = CT_TUPLE;
            break;
        }
            
            /* operation that envolves returning a value */
        case OC_SIZE:
        {
            ctcode = CT_RESULT;
            break;
        }
            
            
        default:
            ctcode = TASK_FAILED;
            break;
    }
    
    return ctcode;
}

/*
 *  Creates a tuple from user input
 */
struct tuple_t* create_tuple_from_input (char *user_input){
    
    char *token;
    char *search = " \"";
    char *elem1 = NULL;
    char *elem2 = NULL;
    char *elem3 = NULL;
    struct tuple_t * tuple_to_send;
    
    // Token will point to comand.
    token = strtok(user_input, search);
    printf("COMANDO = %s\n", token);
    
    // Token will point to elem1.
    token = strtok(NULL, search);
    if (strcmp(token, "*") != 0) {
        elem1 = strdup (token);
    }
    printf("TOKEN - ELEM 1 = %s\n", token);
    printf("ELEM 1 = %s\n", elem1);
    
    // Token will point to elem2.
    token = strtok(NULL, search);
    if (strcmp(token, "*") != 0) {
        elem2 = strdup (token);
    }
    printf("TOKEN - ELEM 2 = %s\n", token);
    printf("ELEM 2 = %s\n", elem2);
    
    // Token will point to elem3.
    token = strtok(NULL, search);
    
    if (strcmp(token, "*") != 0) {
        elem3 = strdup (token);
    }
    printf("TOKEN - ELEM 3 = %s\n", token);
    printf("ELEM 3 = %s\n", elem3);
    
    // creates tuple data
    char* tuple_data[3] = {elem1,elem2,elem3};
    
    //creates new tuple to send
    tuple_to_send = tuple_create2(3, tuple_data);
    
    return tuple_to_send;
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
    
    int exit = 1;
    char input [MAX_MSG]; //vai guardar o comando do utilizador
    char *input_dup;
    int opcode, ctcode;
    
    /* 7.1 Ciclo que trata do user input */
    while (exit != TASK_SUCCEEDED){
        if (fgets (input, MAX_MSG-1, stdin) == NULL){
            return TASK_FAILED;
        }
        
        input_dup = strdup(input);
        printf("TESTE de Input: %s\n", input);
        
        /* 7.2 tratamento do OP_CODE */
        opcode = find_opcode(input_dup);
        printf("OP_CODE --> %d\n", opcode);
        
        /* 7.3 tratamento do CT_CODE */
        ctcode = assign_ctcode (opcode);
        printf("CT_CODE --> %d\n", ctcode);
        
        /* 7.4 tratamento/criação do tuple */
        struct tuple_t *tuple_to_send = create_tuple_from_input (input_dup);
        
        /* 7.5 Criação da mensagem a enviar */
        struct message_t * request_msg = message_create_with(opcode, ctcode, tuple_to_send);
        
        /* 7.6 Envio da mensagem */
        struct message_t * received_msg = network_send_receive(server_to_conect, request_msg);
        
        /* 7.7 Verificação da mensagem de resposta */
        if (response_with_success(request_msg, received_msg) == TASK_FAILED){
            free(input_dup);
            return TASK_FAILED;
        };
        
        //COMO TRATAR DA SAÍDA DO WHILE?
        if (opcode == 0){
            exit = TASK_SUCCEEDED;
        }
        
    }
    
    puts("A terminar sessão...");
    return TASK_SUCCEEDED;
}