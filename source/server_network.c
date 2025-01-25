// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#include "../include/inet-private.h"
#include "../include/server_network.h"
#include "../include/server_network-private.h"
#include "../include/server_skeleton.h"
#include "../include/server_skeleton-private.h"
#include "../include/message-private.h"
#include "../include/table.h"

//novo 3 
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

//novo 4
#include <zookeeper/zookeeper.h>
#include <string.h>
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"


//Proj3
/***********************************************************************************/

pthread_rwlock_t rwlock;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

// static para ficarem privadas so para este script
// usadas para caso de fecho abrupto do servidor
static int l_socket;
static struct table_t *t;

/***********************************************************************************/

// * ZOOKEEPER ou proj4
/************************************************************************* */
//zookeeper info
typedef struct String_vector zoo_string;
static zhandle_t *zh;

//next_node info
zoo_string next_znode;
int next_exists;
char* next_znode_id;
struct rtable_t *next_znode_rtable;

//this node info
char* this_node_path;
int node_id;

//talvez não tenha que estar aqui
char *root_path = "/chain";
char node_path[120] = "";
#define ZDATALEN 1024 * 1024

/************************************************************************* */

typedef struct 
{
    int arg1;
    struct table_t *table;
} ClientThreadArgs;


void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx)
{
    
}

//receives the node and turns into a comparable integer
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

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) 
{
    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
    if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) 
    {
        fprintf(stderr, "Error setting watch at %s!\n", root_path);
    }

    int current_id;
    char* current;
    char* max = "node9999999999";
    for (int i = 0; i < children_list->count; i++)  
    {
        current = children_list->data[i];
        current_id = getNodeId(current);
        if (current_id > node_id && current_id < getNodeId(max)) 
        {
            max = current;
        }
    }

    int compare = strcmp(max, "node9999999999");
    if (compare != 0 && next_znode_id != max)
    {
        if (next_exists)
        {
            rtable_disconnect(next_znode_rtable);
            
        }

        char* buffer = malloc(50);
        int buffer_len = 50;    

        // Construct the path with a `/` separator
        char* max_path = malloc(strlen(root_path) + strlen(max) + 2); // +2 for '/' and '\0'
        if (!max_path) {
            fprintf(stderr, "Memory allocation failed for min_path!\n");
            exit(EXIT_FAILURE);
        }

        sprintf(max_path, "%s/%s", root_path, max);

        if (ZOK != zoo_get(zh, max_path, 0, buffer, &buffer_len, NULL))
        {
            fprintf(stderr, "Error getting data from znode %s!\n", buffer);
            fflush(stderr);
        }

        free(max_path);


        next_exists = 1;
  
        next_znode_id = max;

        printf("Connecting to next node... %s\n", buffer);
        fflush(stdout);
        next_znode_rtable = rtable_connect(buffer);
        if (next_znode_rtable == NULL) 
        {
            fprintf(stderr, "Error connecting to next node!\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        printf("Connected to next node!\n");
        fflush(stdout);

    }
    else if (compare == 0 && next_exists)
    {
        rtable_disconnect(next_znode_rtable);
        next_exists = 0;
        next_znode_id = NULL;
    }
    if (next_znode_id == NULL) 
    {
        next_exists = 0;
        printf("No next node found\n");
        fflush(stdout);
    }
    else 
    {
        printf("Next node found: %s\n", next_znode_id);
        fflush(stdout);
    }
}


void *handle_next_server() 
{
    zoo_string* children_list =	NULL;
    static char *watcher_ctx = "ZooKeeper Data Watcher";
    children_list =	(zoo_string *) malloc(sizeof(zoo_string));

    if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) 
    {
        fprintf(stderr, "Error setting watch at %s!\n", root_path);
    }

    next_exists = 0;
    next_znode_id = NULL;

    //to avoid warnings
    return NULL;
   
}

int server_network_init(short port)
{
    int sockfd;
    struct sockaddr_in server;
    int opt = 1;

    // Initialize read-write lock
    if (pthread_rwlock_init(&rwlock, NULL) != 0) {
        perror("Erro ao inicializar rwlock");
        return -1;
    }

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) 
    {
        perror("Erro ao criar socket");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Erro ao configurar setsockopt");
        close(sockfd);
        return -1;
    }

    // Preenche estrutura server com endereço(s) para associar (bind) à socket 
    server.sin_family = AF_INET;
    server.sin_port = htons(port); // Porta TCP
    server.sin_addr.s_addr = htonl(INADDR_ANY); // Todos os endereços na máquina

    // Faz bind
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    }

    // Esta chamada diz ao SO que esta é uma socket para receber pedidos
    if (listen(sockfd, SOMAXCONN) < 0)
    {
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    }


    // * new 
  
    zh = zookeeper_init(host, my_watcher_func,	2000, 0, NULL, 0); 
	  if (zh == NULL)
    {
		  fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	    exit(EXIT_FAILURE);
	  }

    char *root_path = "/chain";
    strcat(node_path,root_path); 
    strcat(node_path,"/node");
    int this_node_path_len = 1024;
    this_node_path = malloc(this_node_path_len);

    //garantir que /chain existe, senão criamos a /chain
    if (ZNONODE == zoo_exists(zh, root_path, 0, NULL))
    {
        if (ZOK != zoo_create( zh, root_path, NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
            fprintf(stderr, "Error creating chain!\n");
	        exit(EXIT_FAILURE);
        }
	}

    //criar o nosso znode 
    if (ZOK != zoo_create(zh, node_path, "node data", 10, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, this_node_path, this_node_path_len))
    {
        fprintf(stderr, "Error creating znode from path %s!\n", node_path);
    }

    //garantir que znode criado tem o formato correto
    //se sim guaradamos o id do node
    if (strncmp(this_node_path, "/chain/node", strlen("/chain/node")) == 0) 
    {
        char* path = (char*)this_node_path + strlen("/chain/");
        node_id = getNodeId(path);
    } 
    else 
    {
        fprintf(stderr, "Error: node_path format is invalid.\n");
        exit(EXIT_FAILURE);
    }

    //enought to handle the biggest integer
    char buffer[16]; 
    char ip_port[9] = "127.0.0.1";      // Temporary IP                                

    snprintf(buffer, sizeof(buffer), "%s:%d", ip_port, port);
    printf("Buffer: %s\n", buffer);
    fflush(stdout);
    //aqui estamos a por o socket como data do znode criado
    if (ZOK != zoo_set(zh, this_node_path, buffer, sizeof(buffer), -1)) 
    {
        fprintf(stderr, "Error setting data to znode %s!\n", this_node_path);
        exit(EXIT_FAILURE);
    }

    //criação e atualização do next node 
    //isto será feito numa thread á parte
    //se não ouver nenhum node com id superior ao nosso então temos next_node_id = NULL e somos o ponto de leitura
    //se tivermos um next_node_id temos que escrever neste cada vez que recebermos uma mensagem,
    //propagando assim a mensagem recebida
    //perante qualquer criação de um node esta variavel deve ser atualizada
    
    handle_next_server();


    return sockfd;
}

int isWrite(MessageT *msg)
{
    if (msg->opcode == MESSAGE_T__OPCODE__OP_PUT || msg->opcode == MESSAGE_T__OPCODE__OP_DEL)
    {
        return 1;
    }
    return 0;

}

void comecaRead()
{
    if (pthread_rwlock_rdlock(&rwlock))
    {
        perror("Erro ao fazer read lock");
    }
}


void terminaRead()
{
    if (pthread_rwlock_unlock(&rwlock))  
    {
        perror("Erro ao fazer unlock");
    }
}


// Função de controlo de entrada
void comecaWrite()
{
	if (pthread_rwlock_wrlock(&rwlock))
    {
        perror("Erro ao fazer write lock");
    }
}


// Função de controlo de saída
void terminaWrite()
{
    if (pthread_rwlock_unlock(&rwlock))  
    {
        perror("Erro ao fazer unlock");
    }

}

void *clientThread(void *arguments)
{
    ClientThreadArgs *args = (ClientThreadArgs *)arguments;
    int connsockfd = args->arg1;
    struct table_t *table = (struct table_t*) args->table;

    pthread_mutex_lock(&m);
    increment_clients();
    pthread_mutex_unlock(&m);

    MessageT *mensagem;
    struct timeval start, end;
    struct timezone tz;

    while (1)
    {

        mensagem = network_receive(connsockfd);
        if (mensagem == NULL)
        {
            perror("Cliente desconectado ou erro ao receber mensagem");
            break;
        }

        //New part to propagate the message to the next server
        if (next_znode_id) //executes if next_znode_id.count == 1
        { 
            if (network_send(next_znode_rtable->sockfd, mensagem) == -1)
            {
                perror("Erro ao enviar mensagem a outro servidor");
                break;
            }
            fflush(stdout);
        }

        int x = isWrite(mensagem);
        if (x)
        {
            comecaWrite();
        }
        else
        {
            comecaRead();
        }

        gettimeofday(&start, &tz);

        invoke(mensagem, table);

        gettimeofday(&end, &tz);

        pthread_mutex_lock(&m);
        increment_time(end.tv_usec - start.tv_usec);
        pthread_mutex_unlock(&m);

        if (x)
        {
            terminaWrite();
        }
        else
        {
            terminaRead();
        }

        printf("A enviar resposta...\n");
        fflush(stdout);

        if (network_send(connsockfd, mensagem) == -1)
        {
            perror("Erro ao enviar mensagem");
            break;
        }
        printf("Resposta enviada!\n");
        fflush(stdout);


        if (mensagem->opcode != MESSAGE_T__OPCODE__OP_STATS + 1 && mensagem->opcode != MESSAGE_T__OPCODE__OP_ERROR)
        {
            pthread_mutex_lock(&m);
            increment_total_ops();
            pthread_mutex_unlock(&m);
        }
        
        message_t__free_unpacked(mensagem, NULL); 
    }
    close(connsockfd);

    pthread_mutex_lock(&m);
    decrement_clients();
    pthread_mutex_unlock(&m);
    
    printf("Conexão fechada pelo cliente.\n");
    fflush(stdout);

    free(args);

    //Fechar a thread manualmente
    // Mudar pq nao deve ficar assim ?
    pthread_exit(NULL);

}

static void network_terminate()
{
    printf("A terminar servidor...\n");
    fflush(stdout);
    server_network_close(l_socket);
    server_skeleton_destroy(t);
}

int network_main_loop(int listening_socket, struct table_t *table)
{
    l_socket = listening_socket;
    t = table;
    int connsockfd;
    struct sockaddr_in client;
    socklen_t size_client = sizeof(client);
    struct sigaction sa_close_network;
    
    // Terminar o servidor corretamente em caso de CTRL+C
    sa_close_network.sa_handler = network_terminate;
    sa_close_network.sa_flags = 0;
    sigemptyset(&sa_close_network.sa_mask);
    if (sigaction(SIGINT, &sa_close_network, NULL))
    {
        perror("sa_close_network");
        return -1;
    }

    while (1) 
    {
        printf("A espera de conexoes...\n");
        fflush(stdout);
        connsockfd = accept(listening_socket, (struct sockaddr *) &client, &size_client);
        if (connsockfd < 0)
        {
            perror("Erro ao aceitar conexão, a desligar servidor...");
            close(listening_socket);
            return -1;
        }

        ClientThreadArgs *args = malloc(sizeof(ClientThreadArgs));
        args->arg1 = connsockfd;
        args->table = table;
        pthread_t nova;

        if (pthread_create(&nova, NULL, &clientThread, (void *) args) != 0)
        {
            perror("Erro ao criar thread");
            close(connsockfd);
            return -1;
        }

        if (pthread_detach(nova) != 0) {
            perror("Failed to detach thread");
            return -1;
        }

        printf("Conexão aceita. A espera de dados...\n");
        fflush(stdout);
    }

    server_network_close(listening_socket);
    server_skeleton_destroy(table);
    return -1;
}

MessageT *network_receive(int client_socket) 
{
    uint16_t msg_size;
    size_t nbytes;
    ssize_t n;
    
    if (read_all(client_socket, &msg_size, sizeof(uint16_t)) != sizeof(uint16_t)) {
        // caso de quit ele nao envia nada
        return NULL;
    }

    nbytes = (size_t) ntohs(msg_size);
    if (nbytes <= 0) 
    {
        perror("Erro ao ler o tamanho da mensagem");
        return NULL;
    }

    uint8_t *buffer = malloc(nbytes);
    if (buffer == NULL) 
    {
        printf("Erro ao alocar memória para o buffer\n");
        fflush(stdout);
        return NULL;
    }

    n = read_all(client_socket, buffer, nbytes);
    if (n <= 0) 
    {
        perror("Erro ao ler a mensagem do cliente");
        free(buffer);
        return NULL;
    }

    // deserializar a mensagem
    MessageT *msg = message_t__unpack(NULL, nbytes, buffer);

    if (msg == NULL) 
    {
        printf("Erro ao de-serializar a mensagem\n");
        fflush(stdout);
    }

    free(buffer);

    return msg;
}

int network_send(int client_socket, MessageT *msg) {
    size_t msg_size = message_t__get_packed_size(msg);

    // buffer para a mensagem serializada
    uint8_t *buffer = malloc(msg_size);
    if (buffer == NULL) 
    {
        printf("Erro ao alocar memória para o buffer.\n");
        fflush(stdout);
        return -1; 
    }

    // Serializar a mensagem
    message_t__pack(msg, buffer);

    uint16_t msg_size_int = htons((uint16_t)msg_size); // Converter para a ordem de bytes da rede
    if ((write_all(client_socket, &msg_size_int, sizeof(msg_size_int))) != sizeof(msg_size_int)) 
    {
        perror("Erro ao enviar tamanho da mensagem ao cliente");
        free(buffer);
        return -1; 
    }

    int w = write_all(client_socket, buffer, msg_size);
    if (w != msg_size) 
    {
        perror("Erro ao enviar mensagem ao cliente");
        free(buffer);
        return -1; 
    }

    free(buffer);
    
    return 0; 
}

int server_network_close(int socket) 
{
    if (close(socket) < 0) 
    {
        perror("Erro ao fechar socket");
        return -1;
    }

    if (pthread_rwlock_destroy(&rwlock))
    {
        perror("Erro ao destruir rwlock");
        return -1;
    }

    if (pthread_mutex_destroy(&m))
    {
        perror("Erro ao destruir mutex");
        return -1;
    }

    return 0;
}

//new stuff for the main to use
void previous_child_watcher(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx)
{

}


char* get_previous_server()
{
    zoo_string* children_list =	NULL;
    static char *watcher_ctx = "ZooKeeper Data Watcher";
    children_list =	(zoo_string *) malloc(sizeof(zoo_string));

	if (ZOK != zoo_wget_children(zh, root_path, &previous_child_watcher, watcher_ctx, children_list)) 
    {
        fprintf(stderr, "Error setting watch at %s!\n", root_path);
    }

    // * Children List does not comes sorted so we need to find the closest one to our node that is above it

    int current_id;
    char* current;
    char* min = "node0000000000";

    for (int i = 0; i < children_list->count; i++)
    {
        current = children_list->data[i];
        current_id = getNodeId(current);
        if (current_id < node_id && current_id > getNodeId(min))
        {
            min = current;
        }
    }

    //edge case the ser o primeiro node com id 0 e estar a tentar encontrar um anterior que seria ele proprio
    if (strcmp(min, "node0000000000") != 0)
    {
        printf("Previous node found: %s\n", min);
        fflush(stdout);
        char* buffer = malloc(50);
        int buffer_len = 50;

        char* min_path = malloc(strlen(root_path) + strlen(min) + 2); // +2 for '/' and '\0'
        if (!min_path) {
            fprintf(stderr, "Memory allocation failed for min_path!\n");
            exit(EXIT_FAILURE);
        }

        // Construct the path with a `/` separator
        sprintf(min_path, "%s/%s", root_path, min);
        printf("Previous node path: %s\n", min_path);
        fflush(stdout);

        if (ZOK != zoo_get(zh, min_path, 0, buffer, &buffer_len, NULL))
        {
            fprintf(stderr, "Error getting data from znode %s!\n", min);
            fflush(stderr);
            return "none";
        }
        free(min_path);

        return buffer;
    }

    printf("No previous node found\n");
    fflush(stdout);

    return "none";

}