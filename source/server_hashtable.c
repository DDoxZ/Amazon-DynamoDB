// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "../include/server_network.h"
#include "../include/server_network-private.h"
#include "../include/server_skeleton-private.h"
#include "../include/server_skeleton.h"
#include "../include/htmessages.pb-c.h"

//proj 4
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/client_network.h"
#include "../include/table.h"
#include <string.h>

//#include "../source/server_network.c"

char* host;

int testServerInput(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Uso: server_hashtable <zookeeper ip:port> <port> <n_lists>\n");
        fflush(stdout);
        printf("Examplo de uso: ./server_hashtable localhost:2181 12345 3\n");
        fflush(stdout);
        return -1;
    } 
    return 0;
}


int main(int argc, char** argv)
{
    signal(SIGPIPE, SIG_IGN);
    // Verificar input de ligacao do servidor
    if (testServerInput(argc, argv) < 0) return -1;

    // Inicializar o servidor
    host = argv[1];
    int sockfd = server_network_init((short) atoi(argv[2]));
    if (sockfd < 0)
    {
        printf("[Erro-Servidor 0]: Inicializacao do servidor!\n");
        fflush(stdout);
        return -1;
    }
    
    // Inicializar a tabela
    
    char* previous_server =  get_previous_server();
    printf("Previous server: %s\n", previous_server);
    fflush(stdout);

    struct table_t *table = server_skeleton_init(atoi(argv[3]));
    if (table == NULL)
    {
        printf("[Erro-Servidor 1]: Inicializacao da tabela!\n");
        fflush(stdout);
        return -1;
    }

   if (strcmp(previous_server, "none") != 0)
    {              
        // Combine IP:Port and integer into result
        struct rtable_t *rtable = rtable_connect(previous_server);

        struct entry_t ** entries = rtable_get_table(rtable);
        if (entries != NULL)
        {
            decrement_total_ops();
        }
        for (int i = 0; entries[i] != NULL; i++)
        {
            if (table_put(table, entries[i]->key,entries[i]->value))
            {
                printf("[Erro-Servidor 1]: Erro ao escrever na tabela!\n");
                fflush(stdout);
            }
        }

        if (rtable_disconnect(rtable) == -1)
        {
            printf("[Erro-Servidor 1]: Erro ao fechar ligação ao servidor anterior!\n");
                fflush(stdout);
        }
        

    }

    printf("Servidor inicializado!\n");
    fflush(stdout);

    // Loop de funcionamento do servidor
    if (network_main_loop(sockfd, table) == -1)
    {
        printf("[Erro-Servidor 2]: Loop de funcionamento do servidor!\n");
        fflush(stdout);
        return -1;
    }

}