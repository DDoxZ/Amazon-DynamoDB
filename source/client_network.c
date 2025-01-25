// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#include <zookeeper/zookeeper.h>

#include "client_stub-private.h"
#include "inet-private.h"
#include "client_network.h"
#include "htmessages.pb-c.h"
#include "message-private.h"


int network_connect(struct rtable_t *rtable) 
{
    int sockfd;
    struct sockaddr_in server;

    //abre socket do cliente
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    // Preenche estrutura server para estabelecer conexão
    server.sin_family = AF_INET;
    server.sin_port = htons(rtable->server_port);
    if (inet_pton(AF_INET, rtable->server_address, &server.sin_addr) < 1) 
    {
        printf("Erro ao converter IP\n");
        fflush(stdout);
        close(sockfd);
        return -1;
    }

    // Estabelece conexão com o servidor definido em server
    if (connect(sockfd,(struct sockaddr *)&server, sizeof(server)) < 0) 
    {
        perror("Erro ao conectar-se ao servidor");
        close(sockfd);
        return -1;
    }

    rtable->sockfd = sockfd;

    
    return 0;

}


MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) 
{
    size_t msg_size = message_t__get_packed_size(msg);
    int nbytes;
    
    // buffer para a mensagem serializada
    uint8_t *buffer = malloc(msg_size);
    if (buffer == NULL) 
    {
        printf("Erro ao alocar memória para o buffer.\n");
        fflush(stdout);
        return NULL;
    }

    //serializar
    message_t__pack(msg, buffer);

    uint16_t msg_size_network = htons((uint16_t)msg_size);

    //envia o tamanho da mensagem a ser enviada 
    if (write_all(rtable->sockfd, &msg_size_network, sizeof(msg_size_network)) != sizeof(msg_size_network)) 
    {
        perror("Erro ao enviar o tamanho da mensagem ao servidor");
        free(buffer);
        return NULL;
    }

    //envia a mensagem serializada para o servidor
    if((nbytes = write_all(rtable->sockfd,buffer,msg_size)) != msg_size)
    {
        perror("Erro ao enviar dados ao servidor");
        close(rtable->sockfd);
        free(buffer);
        return NULL;
    }
    free(buffer);

    uint16_t response_size_network;

    //recebe o tamanho da mensagem do servidor
    ssize_t nbytes_read = read_all(rtable->sockfd, &response_size_network, sizeof(uint16_t));
    if (nbytes_read != sizeof(response_size_network)) 
    {
        perror("Erro ao receber o tamanho da resposta do servidor");
        return NULL;
    }

    size_t response_size = (size_t) ntohs(response_size_network);

    //buffer para colocar mensagem recebida
    uint8_t *response_buffer = malloc(response_size);
    if (response_buffer == NULL) 
    {
        printf("Erro ao alocar memória para o buffer de resposta.\n");
        fflush(stdout);
        return NULL;
    }

    //coloca mensagem do servidor num buffer
    if (read_all(rtable->sockfd, response_buffer, response_size) != response_size) 
    {
        perror("Erro ao receber a resposta do servidor");
        free(response_buffer);
        return NULL;
    }

    //deserializar mensagem
    MessageT *response_msg = message_t__unpack(NULL, response_size, response_buffer);
    if (response_msg == NULL) 
    {
        fprintf(stderr, "Erro ao deserializar a resposta do servidor.\n");
        fflush(stdout);
        free(response_buffer);
        return NULL;
    }

    free(response_buffer);
    return response_msg;
}

int network_close(struct rtable_t *rtable) 
{
    if (close(rtable->sockfd) < 0) 
    {
        perror("Erro ao fechar socket");
        return -1;
    }
    return 0;
}