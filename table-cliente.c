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
#include "network_utils.h"
#include "client_stub.h"


/*
 *  Acts according with user command
 *  returns 0 if success, -1 otherwise
 */
int proceed_with_command (int opcode, struct rtable_connection * system_init, void *message_content){
    
    int taskSuccess = TASK_FAILED;
    int keep_tuples = -1; //apenas para inicializar
    int one_or_all = -1; //apenas para inicializar
    struct rtable_t * rtable_switch = system_init->rtable_switch;
    struct rtable_t * rtable_replica = system_init->rtable_replica;
    
    
    switch (opcode) {
        case OC_SIZE:
            taskSuccess = rtable_size(rtable_replica);
            if (taskSuccess == TASK_FAILED){
                return TASK_FAILED;
            }
            break;
            
        case OC_OUT:
            taskSuccess = rtable_out(rtable_switch, message_content);
            
            /*** REVER MUITO BEM A SITUAÇÂO DO SWITCH ***/
            //Verifica se a ligação ao SWITCH está ativa (PROJETO 5)
            if (taskSuccess == TASK_FAILED && socket_is_closed(rtable_switch->server_to_connect.socketfd)){
                puts ("SWITCH SOCKET IS CLOSED!");
                //faz unbind do switch
                taskSuccess = rtable_unbind(system_init->rtable_switch);
                if (taskSuccess == TASK_FAILED)
                    puts ("UNABLE TO UNBIND RTABLE_SWITCH!");
                
                char * new_switch_address = strdup (rtable_report(system_init));
                if (new_switch_address == NULL){
                    free(new_switch_address);
                    return TASK_FAILED;
                }
                free(new_switch_address);
                taskSuccess = rtable_out(rtable_switch, message_content);
            }
            break;
            
        case OC_IN:
        case OC_IN_ALL:
        case OC_COPY:
        case OC_COPY_ALL:
            keep_tuples = opcode == OC_IN || opcode == OC_IN_ALL ? DONT_KEEP_AT_ORIGIN : KEEP_AT_ORIGIN;
            one_or_all = opcode == OC_IN || opcode == OC_COPY;
            taskSuccess = TASK_SUCCEEDED;
            break;
        default:
            taskSuccess = TASK_FAILED;
            break;
    }
    
    //GETTER REQUESTS
    if ((opcode != OC_SIZE && opcode != OC_OUT) && ((one_or_all != -1 ) && (keep_tuples != -1))){
        struct tuple_t ** received_tuples = rtable_get(rtable_switch, message_content, keep_tuples, one_or_all);
        
        /*** REVER MUITO BEM A SITUAÇÂO DO SWITCH ***/
        //Verifica se a ligação ao SWITCH está ativa (PROJETO 5)
        if (received_tuples == NULL && socket_is_closed(rtable_switch->server_to_connect.socketfd)){
            puts ("SWITCH SOCKET IS CLOSED!");
            //faz unbind do switch
            taskSuccess = rtable_unbind(system_init->rtable_switch);
            
            char * new_switch_address = strdup (rtable_report(system_init));
            if (new_switch_address == NULL){
                free(new_switch_address);
                return TASK_FAILED;
            }
            free(new_switch_address);
            received_tuples = rtable_get(rtable_switch, message_content, keep_tuples, one_or_all);
            if (received_tuples == NULL){
                puts ("FAILLED TO RECEIVE TUPLES!");
                taskSuccess = TASK_FAILED;
            }
        }
    }
    
    return taskSuccess;
}

/*
 * Processes user command
 * Returns 0 if sucess, -1 otherwise
 */
int process_command (const char* command, struct rtable_connection * system_init){
    
    int taskSuccess = TASK_FAILED;
    int opcode = find_opcode(command);
    int ctype = assign_ctype(opcode);
    
    void * message_content = NULL;
    
    //defines message type content - CT_TUPLE
    if (ctype == CT_TUPLE){
        message_content = create_tuple_from_input(command);
    }
    //defines message type content - CT_RESULT
    if( ctype == CT_RESULT){
        int resultValue = 0;
        message_content = &resultValue;
    }
    
    taskSuccess = proceed_with_command (opcode, system_init, message_content);
    
    return taskSuccess;
}

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

/*
 *  Message if command is invalid
 */
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

int main(int argc , char *argv[]) {
    
    int taskSuccess = TASK_SUCCEEDED;
    
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
    
    
    /* 2. Obter os diversos endereços e inicializar a rtable */
    char * file_path = strdup(argv[1]);
    
    struct rtable_connection * system_init;
    
    system_init = rtable_init(file_path); //vai inicializar a estrutura rtable_connection
    
    if (system_init == NULL){
        taskSuccess = TASK_FAILED;
        puts ("FAILLED DO SYSTEM_INIT!");
    }
    
    //ligação foi bem sucessida
    if (taskSuccess == TASK_SUCCEEDED){
        
        /* 3. LÊ COMANDO DO UTILIZADOR E FAZ CONSULTAS À TABELA */
        while ( keepGoing ) {
            
            puts("\n---- Introduzir comando ----");
            //reads user command
            if (fgets (input, MAX_MSG-1, stdin) == NULL) {
                free(file_path);
                return TASK_FAILED;
            }
            
            // Processes user command according with objectives - OUT, GET, SIZE
            int command_opcode = find_opcode(input);
            
            // User wants to quit
            if (command_opcode == OC_QUIT) {
                keepGoing = NO;
            }
            // User instruction is wrong
            else if ( command_opcode == OC_DOESNT_EXIST ) {
                invalid_command();
            }
            
            else{
                /******* O comando foi correcto e vai proceder à consulta da tabela *******/
                taskSuccess = process_command(input, system_init); //processa o pedido do utilizador
                
                //se pedido falhou e não é para repetir pedido tenta terminar de forma controlada
                if (taskSuccess == TASK_FAILED){
                    puts("\t--- failed to consult table\n");
                }
            } //termina o processamento do user_command
        } //sai do while(keepGoing)
    }
    
    //para uma terminação controlada
    puts("A terminar sessão...");
    sleep(2); //para fica mais bonito
    
    free(file_path);
    
    if (system_init != NULL){
        taskSuccess = rtable_disconnect(system_init);
    }
    
    if (taskSuccess == TASK_FAILED){
        return TASK_FAILED;
    }
    
    puts("\nSessão terminada!\n");
    //termina processo com sinal de successo
    return taskSuccess;
}