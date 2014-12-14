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

#define NONE_CONSULTED 0
#define SWITCH_CONSULTED 1
#define REPLICA_CONSULTED 2


/*
 *  Acts according with user command
 *  returns 0 if success, -1 otherwise
 */
int proceed_with_command (int opcode, struct rtable_connection * system_rtable_connection, void *message_content)
{
    
    int taskSuccess = FAILED;
    struct rtable_t * rtable_switch = system_rtable_connection->rtable_switch;
    struct rtable_t * rtable_replica = system_rtable_connection->rtable_replica;
    
    struct  rtable_t * consulted_rtable = NULL;
    
    switch (opcode) {
            
        case OC_SIZE:
            taskSuccess = rtable_size(rtable_replica);
            consulted_rtable = rtable_replica;
            break;
            
        case OC_OUT:
            taskSuccess = rtable_out(rtable_switch, message_content);
            consulted_rtable = rtable_switch;
            break;
            
        case OC_COPY:
        case OC_COPY_ALL:
        case OC_IN:
        case OC_IN_ALL:
        {
            int whatToDoWithTheTuples = opcode == OC_COPY || opcode == OC_COPY_ALL ?
            KEEP_AT_ORIGIN : DONT_KEEP_AT_ORIGIN;
            int one_or_all = opcode == OC_COPY || opcode == OC_IN;
            int justReads = whatToDoWithTheTuples == KEEP_AT_ORIGIN;
            /* which rtable was consulted? */
            consulted_rtable = justReads ? rtable_replica : rtable_switch;
            /* gets the tuples depending on the way to get them (by just reading or by taking them from the rtable */
            struct tuple_t **received_tuples =  rtable_get(consulted_rtable,  message_content, whatToDoWithTheTuples, one_or_all);
            
            taskSuccess = received_tuples != NULL;
            break;
        }
            
        default:
            taskSuccess = FAILED;
            break;
            
    } 
    
    /*** checks if some error happened because of unreachable peers and reconnects and tries again ***/
    
    if ( taskSuccess == FAILED && consulted_rtable != NULL ) {
        puts(" if ( taskSuccess == FAILED && consulted_rtable != NULL");
        /* which rtable was consulted */
        int switchWasTheConsulted = consulted_rtable == rtable_switch;
        
//        if ( socket_is_closed(consulted_rtable->server_to_connect.socketfd) ) {
//            puts(" socket is closed");
            int rebindSuccess = rtable_connection_server_rebind(system_rtable_connection, switchWasTheConsulted);
            
            if ( rebindSuccess == SUCCEEDED ) {
                return proceed_with_command(opcode, system_rtable_connection, message_content );
            }
            else {
                puts("\t --- error consulting remote table and didnt manage to solve it.");
            }
       // }
    }
    
    
    
    return taskSuccess;
}

/*
 * Processes user command
 * Returns 0 if sucess, -1 otherwise
 */
int process_command (const char* command, struct rtable_connection * system_rtable_connection){
    
    int taskSuccess = FAILED;
    int opcode = find_opcode_as_string(command);
    int ctype = assign_ctype(opcode, NO);
    
    void * message_content = NULL;
    
    //defines message type content - CT_TUPLE
    if (ctype == CT_TUPLE) {
        message_content = create_tuple_from_input(command);
    }
    //defines message type content - CT_RESULT
    if( ctype == CT_RESULT){
        int resultValue = 0;
        message_content = &resultValue;
    }
    
    taskSuccess = proceed_with_command (opcode, system_rtable_connection, message_content);
    
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
        return FAILED;
    }
    return SUCCEEDED;
}

int main(int argc , char *argv[]) {
    
    int taskSuccess = SUCCEEDED;
    
    /* 0. SIGPIPE Handling */
    struct sigaction s;
    //what must do with a signal - ignore
    s.sa_handler = SIG_IGN;
    //set what to do when gets the SIGPIPE
    sigaction(SIGPIPE, &s, NULL);
    
    /* 1. Testar input de utilizador*/
    if ( test_input (argc) == FAILED ) return FAILED;
    
    //to keep the active
    int keepGoing = YES;
    //para guardar o comando do utilizador
    char input [MAX_MSG];
    
    
    /* 2. Obter os diversos endereços e inicializar a rtable */
    char * file_path = strdup(argv[1]);
    
    struct rtable_connection * system_rtable_connection;
    
    system_rtable_connection = rtable_init(file_path); //vai inicializar a estrutura rtable_connection
    
    if (system_rtable_connection == NULL){
        taskSuccess = FAILED;
        puts ("FAILLED DO system_rtable_connection!");
    }
    
    //ligação foi bem sucessida
    if (taskSuccess == SUCCEEDED){
        
        /* 3. LÊ COMANDO DO UTILIZADOR E FAZ CONSULTAS À TABELA */
        while ( keepGoing ) {
            
            puts("\n---- Introduzir comando ----");
            //reads user command
            if (fgets (input, MAX_MSG-1, stdin) == NULL) {
                free(file_path);
                return FAILED;
            }
            
            // Processes user command according with objectives - OUT, GET, SIZE
            int command_opcode = find_opcode_as_string(input);
            
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
                taskSuccess = process_command(input, system_rtable_connection);
                //se pedido falhou discarta este pedido e pede outro
                if (taskSuccess == FAILED){
                    puts("\t--- failed to consult table\n");
                }
            } //termina o processamento do user_command
        } //sai do while(keepGoing)
    }
    
    //para uma terminação controlada
    puts("A terminar sessão...");
    sleep(2); //para fica mais bonito
    
    free(file_path);
    
    if (system_rtable_connection != NULL){
        taskSuccess = rtable_disconnect(system_rtable_connection);
    }
    
    if (taskSuccess == FAILED){
        return FAILED;
    }
    
    puts("\nSessão terminada!\n");
    //termina processo com sinal de successo
    return taskSuccess;
}