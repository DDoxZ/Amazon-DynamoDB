// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <zookeeper/zookeeper.h>

#include "block.h"
#include "entry.h"
#include "client_stub.h"
#include "stats.h"

#define comandoMaxSize 1024

typedef struct String_vector zoo_string;

// static struct rtable_t *rtable = NULL;
static struct rtable_t *rtable_head;
static struct rtable_t *rtable_tail;
static zhandle_t *zh;
char *root_path = "/chain";
char *min_node, *max_node;
int changed_min, changed_max;


int getNodeId(char* str)
{
    char* begining = "node";
    if (strncmp(str, begining, strlen(begining)) != 0)
    {
        perror("Erro ao converter id de node");
        return -1;
    }
    
    return atoi(str + strlen(begining));
}

int handle_client_connection();

static void child_watcher(zhandle_t *zh, int type, int state, const char *zpath, void *watcher_ctx) 
{
    handle_client_connection();
}

int handle_client_connection()
{
    zoo_string* children_list =	NULL;
    static char *watcher_ctx = "ZooKeeper Data Watcher";
    children_list =	(zoo_string *) malloc(sizeof(zoo_string));
    char min_address[512] = {0}, max_address[512] = {0};
    char buffer[512];
    int buffer_len = sizeof(buffer);
    char *min, *max;

    if (min_node == NULL || max_node == NULL) 
    {
        min_node = malloc(sizeof(char) * 20);
        max_node = malloc(sizeof(char) * 20);
        strcpy(min_node, "node9999999998");
        strcpy(max_node, "node9999999999");
        if (min_node == NULL || max_node == NULL) 
        {
            printf("min_node / max_node mem alloc error!\n");
            return 1;
        }
    }

    if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) {
        fprintf(stderr, "Error setting watch at %s!\n", root_path);
    }

    min = children_list->data[0];
    max = children_list->data[0];

    for (int i = 0; i < children_list->count; i++)  
    {
        char* current = children_list->data[i];
        if (getNodeId(current) > getNodeId(max)) max = current;
        if (getNodeId(current) < getNodeId(min)) min = current;
        
    }

    changed_min = getNodeId(min_node) != getNodeId(min);
    changed_max = getNodeId(max_node) != getNodeId(max);

    strcpy(min_node, min);
    strcpy(max_node, max);

    if (changed_min) {
        char* min_path = malloc(strlen(root_path) + strlen(min_node) + 2);
        sprintf(min_path, "%s/%s", root_path, min);
        if (zoo_get(zh, min_path, 0, buffer, &buffer_len, NULL) == ZOK) {
            buffer[buffer_len] = '\0'; 
            strncpy(min_address, buffer, sizeof(min_address));
        }

        rtable_disconnect(rtable_head);
        rtable_head = rtable_connect(min_address);
        if (rtable_head == NULL) {
            printf("Error connecting to new head\n");
            fflush(stdout);
            return 1;
        }
        free(min_path);
    }
    if (changed_max) {
        char* max_path = malloc(strlen(root_path) + strlen(max_node) + 2);
        sprintf(max_path, "%s/%s", root_path, max);
        if (zoo_get(zh, max_path, 0, buffer, &buffer_len, NULL) == ZOK) {
            buffer[buffer_len] = '\0'; 
            strncpy(max_address, buffer, sizeof(max_address));
        }
        rtable_disconnect(rtable_tail);
        rtable_tail = rtable_connect(max_address);
        if (rtable_tail == NULL) {
            printf("Error connecting to new tail\n");
            fflush(stdout);
            return 1;
        }
        free(max_path);
    }

    free(children_list);
    return 0;
}


void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx)
{
    
}

void abrupt_disconnect()
{
    if (rtable_head != NULL && rtable_tail)
    {
        if(rtable_disconnect(rtable_head) != 0)
        {
            printf("[Erro-Cliente 9]: Terminar ligacao cliente-servidor!\n");
            fflush(stdout);
        }

        if(rtable_disconnect(rtable_tail) != 0)
        {
            printf("[Erro-Cliente 9]: Terminar ligacao cliente-servidor!\n");
            fflush(stdout);
        }
        printf("A terminar a ligacao entre o cliente e servidor...\n");
        fflush(stdout);
    }
    free(min_node);
    free(max_node);
    zookeeper_close(zh);
    exit(0);
}

int testClientInput(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Uso: client_hashtable <server>:<port>\n");
        fflush(stdout);
        printf("Examplo de uso: ./client_hashtable localhost:2181\n");
        fflush(stdout);
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{ 
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, abrupt_disconnect);
    // Verificar input de ligacao ao servidor
    if (testClientInput(argc, argv) < 0)
        return -1;

    // Estabelecer ligacao ao servidor
    zh = zookeeper_init(argv[1], my_watcher_func,	2000, 0, NULL, 0);
    handle_client_connection();

    //rtable = rtable_connect(argv[1]);
    if (rtable_head == NULL)
    {
        printf("[Erro-Cliente 0]: Ligacao entre cliente e servidor!\n");
        fflush(stdout);
        return -1;
    }

    if (rtable_tail == NULL)
    {
        printf("[Erro-Cliente 0]: Ligacao entre cliente e servidor!\n");
        fflush(stdout);
        return -1;
    }

    printf("Cliente inicializado e ligado ao servidor!\n");
    fflush(stdout);

    // Ler e tratar dos comandos/pedidos do utilizador
    while (1)
    {
        char comando[comandoMaxSize];

        printf("Insira um comando: ");
        fflush(stdout);
        if (fgets(comando, comandoMaxSize, stdin) == NULL)
        {
            printf("[Erro-Cliente 1]: Leitura do comando!\n");
            fflush(stdout);
            continue;
        }

        // Substituir o \n por \0 vindo do fgets
        comando[strcspn(comando, "\n")] = '\0';

        // Se for null somente pedimos outra vez (strcmp n pode receber null)
        char *token = strtok(comando, " ");
        if (token == NULL) 
        {
            continue;
        }

        if (strcmp(token, "put") == 0)
        {   
            char *key = strtok(NULL, " ");    // NULL para continuar da ultima posicao da ref passada
            char *value = strtok(NULL, "\0"); // ate \n para ler o value todo com espacos
            if (key == NULL || value == NULL)
            {
                printf("Comando invalido!\n");
                fflush(stdout);
                continue;
            }
            char *key_copy = strdup(key);
            char *value_copy = strdup(value);
            struct block_t *value_block = block_create(sizeof(value_copy), value_copy);
            struct entry_t *entry = entry_create(key_copy, value_block);
            if (rtable_put(rtable_head, entry) == 0)
            {
                printf("Entrada adicionada/substituida com sucesso!\n");
                fflush(stdout);
            }
            else
            {
                printf("[Erro-Cliente 2]: Adicionar/substituir elemento!\n");
                fflush(stdout);
            }
            entry_destroy(entry);
        }
        else if (strcmp(token, "get") == 0)
        {
            char *key = strtok(NULL, "\0");
            struct block_t *block = rtable_get(rtable_tail, key);
            if (block != NULL)
            {   
                if (block->data == NULL)
                {
                    printf("Valor associado a chave %s: NULL\n", key);
                    fflush(stdout);
                }
                else 
                {
                    printf("Valor associado a chave %s: %s\n", key, (char *)block->data);
                    fflush(stdout);
                }
            }
            else
            {
                printf("[Erro-Cliente 3]: Obter bloco da entrada da tabela!\n");
                fflush(stdout);
            }
            block_destroy(block);
        }
        else if (strcmp(token, "del") == 0)
        {
            char *key = strtok(NULL, "\0");
            if (rtable_del(rtable_head, key) == 0)
            {
                printf("Elemento removido da tabela com sucesso!\n");
                fflush(stdout);
            }
            else
            {
                printf("[Erro-Cliente 4]: Remover entrada da tabela!\n");
                fflush(stdout);
            }
        }
        else if (strcmp(token, "size") == 0)
        {
            int size = rtable_size(rtable_tail);
            if (size != -1)
            {
                printf("Tamanho da tabela: %d\n", size);
                fflush(stdout);
            }
            else
            {
                printf("[Erro-Cliente 5]: Obter tamanho da tabela!\n");
                fflush(stdout);
            }
        }
        else if (strcmp(token, "getkeys") == 0)
        {
            char **keys = rtable_get_keys(rtable_tail);
            if (keys != NULL)
            {
                printf("Chaves da tabela: \n");
                fflush(stdout);
                for (int i = 0; keys[i] != NULL; i++)
                {
                    printf("Chave: %s \n", keys[i]);
                    fflush(stdout);
                }
                rtable_free_keys(keys);
            }
            else
            {
                printf("[Erro-Cliente 6]: Obter chaves da tabela!\n");
                fflush(stdout);
            }
        }
        else if (strcmp(token, "gettable") == 0)
        {
            struct entry_t **table_entries = rtable_get_table(rtable_tail);
            if (table_entries != NULL)
            {
                printf("Conteudo da tabela: \n");
                fflush(stdout);
                for (int i = 0; table_entries[i] != NULL; i++)
                {
                    if (table_entries[i]->key == NULL) 
                    {
                        printf("Key is NULL\n");
                    } 
                    else 
                    {
                        printf("Chave: %s Valor: %s \n", table_entries[i]->key, (char *)table_entries[i]->value->data);
                    }
                    fflush(stdout);
                }
                rtable_free_entries(table_entries);
            }
            else
            {
                printf("[Erro-Cliente 7]: Obter conteudo da tabela!\n");
                fflush(stdout);
            }
        }

        else if (strcmp(token, "stats") == 0) {
            struct statistics_t *stats = rtable_stats(rtable_tail);

            printf("Total operations: %d\n", stats->total_ops); 
            printf("Total operation time: %d\n",stats->total_op_time);
            printf("Connected clients: %d\n", stats->connected_clients);

            free(stats);
        }
        else if (strcmp(token, "quit") == 0)
        {
            if (rtable_disconnect(rtable_head) == 0)
            {
                printf("A terminar a ligacao entre o cliente e servidor...\n");
                fflush(stdout);
                printf("Ligacao entre cliente e servidor terminada com sucesso!\n");
                fflush(stdout);
                break;
            }
            else
            {
                printf("[Erro-Cliente 8]: Terminar ligacao cliente-servidor!\n");
                fflush(stdout);
            }

            if (rtable_disconnect(rtable_tail) == 0)
            {
                printf("A terminar a ligacao entre o cliente e servidor...\n");
                fflush(stdout);
                printf("Ligacao entre cliente e servidor terminada com sucesso!\n");
                fflush(stdout);
                break;
            }
            else
            {
                printf("[Erro-Cliente 8]: Terminar ligacao cliente-servidor!\n");
                fflush(stdout);
            }
            free(min_node);
            free(max_node);
            zookeeper_close(zh);
        }
        else
        {
            printf("Comando invalido!\n");
            fflush(stdout);
            char comandosValidos[] = "put <key> <value>\n"
                                     "get <key>\n"
                                     "del <key>\n"
                                     "size\n"
                                     "getkeys\n"
                                     "gettable\n"
                                     "stats"
                                     "quit\n";
            printf("Comandos validos:\n%s", comandosValidos);
            fflush(stdout);
        }
    }
}

