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
#include "define-utils.h" //definições de constantes



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
 * tests if input is a valid IP
 */
int is_number (char * stringWithNumber ) {
    char *ptr;
    int value = (int)strtol(stringWithNumber, &ptr, 10);
    return value;
}

/*
 Get ip from domain name
 */
int hostname_to_ip(char * hostname , char* ip){
    
    struct hostent *host_entry;
    struct in_addr **addr_list;
    int i;
    
    if ( (host_entry = gethostbyname( hostname ) ) == NULL){
        // get the host info
        herror("HOSTNAME TO IP ERROR: ");
        return TASK_FAILED;
    }
    
    addr_list = (struct in_addr **) host_entry->h_addr_list;
    
    for(i = 0; addr_list[i] != NULL; i++){
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return ip[i];
    }
    
    return TASK_FAILED;
}

/*
 *  Get port number from user input
 */
int reads_server_portnumber ( const char * stringWithPortNumber ) {
    int port_number = atoi(stringWithPortNumber);
    printf("Get port number from user input --> Port: %d\n", port_number);
    
    return port_number;
}

/*
 *  Verifies if port number is valid
 */
int portnumber_is_invalid (int portNumber ) {
    return (portNumber <= 0 || ((portNumber >=1 && portNumber<=1023) || (portNumber >=49152 && portNumber<=65535)));
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
        return TASK_SUCCEDED;
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
    
    char *server_address = argv[1]; //guarda valor de IP ou endereço de servidor
    char ip[100]; //se for wwww.example.com guarda o IP
	char *port_from_host; //guarda o valor do PORTO
    
    /* 2. Testar se é IP ou Endereço*/
    puts("2. Testar se é IP ou HOSTNAME\n");
    if (is_number (server_address) == 0){
        /* 2.1 Se não for IP resolve endereço para IP*/
        puts("2.1 É um HOSTNAME!");
        puts ("A resolver endereço...\n");
        char *hostname = strdup(argv[1]);
        hostname = strtok(hostname, ":");
        hostname_to_ip(hostname , ip);
        //        printf("HOSTNAME: %s\n", strtok(hostname, ":"));
        //        printf("%s resolved to %s\n" , hostname , ip);
    }
    
    else{
        puts("2. É um IP!");
    }
    
    /* 3. Copia porto do servidor*/
    port_from_host = strtok(server_address, ":"); //gets the host address porque sim, para passar à frente!
    //    printf("HOSTNAME from port_from_host: %s\n", port_from_host);
    
    port_from_host = strtok(NULL,":"); //gets port server
    //    printf("PORT from port_from_host: %s\n", port_from_host);
    
    int server_port = reads_server_portnumber(port_from_host);
    //    printf("Server Port number is %d\n", server_port);
    
    /* 4. Verifica a validade do valor do porto*/
    if ( portnumber_is_invalid(server_port) ) {
        invalid_port_number_message();
        return TASK_FAILED;
    }
    
    /* 5. Imprime IP e Porto do Servidor*/
    printf("Server Address is %s\n", server_address);
    printf("Server Port is %d\n", server_port);
    printf("\n");
    
    /* 6. Ligação ao SERVIDOR */
    
    printf("Ligacao iniciada a SERVIDOR: %s no PORTO: %d\n", server_address, server_port);
    struct server_t *server_to_conect = network_connect(server_address);
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
    while (exit != TASK_SUCCEDED){
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
            exit = TASK_SUCCEDED;
        }
        
    }
    
    puts("A terminar sessão...");
    return TASK_SUCCEDED;
}