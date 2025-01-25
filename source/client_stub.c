// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#include "../include/block.h"
#include "../include/entry.h"
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/client_network.h"
#include "stats.h"
#include <stddef.h> //NULL
#include <string.h> // memcpy
#include <sys/socket.h> // socket, AF_INET, SOCK_STREAM
#include <netinet/in.h> // struct sockaddr_in, htons
#include "htmessages.pb-c.h"
#include <stdlib.h>
#include <stdio.h> // for stdout


int getIndexOf(char *address_port, char c)
{
    for (int i = 0; i < strlen(address_port); i++)
    {
        if (address_port[i] == c)
        {
            return i;
        }
    }
    return -1;
}

struct rtable_t *rtable_connect(char *address_port)
{
    if (address_port == NULL)
    {
        return NULL;
    }

    struct rtable_t *rtable = (struct rtable_t *) malloc(sizeof(struct rtable_t)); 
    if (rtable == NULL)
    {
        return NULL;
    }

    int index = getIndexOf(address_port, ':');
    if (index == -1)
    {
        free(rtable);
        return NULL;
    }

    rtable->server_address = (char *) malloc(index + 1);
    if (rtable->server_address == NULL)
    {
        free(rtable);
        return NULL;
    }

    strncpy(rtable->server_address, address_port, index);
    rtable->server_address[index] = '\0';
    rtable->server_port = atoi(address_port + index + 1);
    if (rtable->server_port < 0)
    {
        free(rtable->server_address);
        free(rtable);
        return NULL;
    }

    if (network_connect(rtable) == -1)
    {
        free(rtable->server_address);
        free(rtable);
        return NULL;
    }

    return rtable;
}

int rtable_disconnect(struct rtable_t *rtable)
{
    if (rtable == NULL)
    {
        return -1;
    }

    if (network_close(rtable) == -1)
    {
        free(rtable->server_address);
        free(rtable);
        return -1;
    }
    
    free(rtable->server_address);
    free(rtable);

    return 0;
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry)
{
    if(rtable == NULL || entry == NULL)
    {
        return -1;
    }

    MessageT message;
    message_t__init(&message); 
    message.opcode = MESSAGE_T__OPCODE__OP_PUT;
    message.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    EntryT entry_t;
    entry_t__init(&entry_t);
    entry_t.key = strdup(entry->key);
    entry_t.value.len = entry->value->datasize;                         
    entry_t.value.data = (uint8_t *)malloc(entry_t.value.len);
    if (entry_t.value.data == NULL)
    {
        free(entry_t.key);
        return -1;
    }
    
    memcpy(entry_t.value.data, entry->value->data, entry_t.value.len);
  
    message.entry = &entry_t;
    
    MessageT *message_answer = network_send_receive(rtable, &message);
  
    if (message_answer == NULL)
    {
        free(entry_t.value.data);
        free(entry_t.key);
        return -1;
    }
 
    if (message_answer->opcode == MESSAGE_T__OPCODE__OP_PUT + 1 &&
        message_answer->c_type == MESSAGE_T__C_TYPE__CT_NONE)
    {
        message_t__free_unpacked(message_answer, NULL);
        free(entry_t.value.data);
        free(entry_t.key);
        return 0;
    }

    message_t__free_unpacked(message_answer, NULL);
    free(entry_t.value.data);
    free(entry_t.key);
    return -1;
}

struct block_t *rtable_get(struct rtable_t *rtable, char *key)
{
    if (rtable == NULL || key == NULL)
    {
        return NULL;
    }

    MessageT message;
    message_t__init(&message);
    message.opcode = MESSAGE_T__OPCODE__OP_GET;
    message.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    message.key = strdup(key);
    if (message.key == NULL)
    {
        free(message.key);
        return NULL;
    }

    MessageT *message_answer = network_send_receive(rtable, &message);
    if (message_answer == NULL)
    {
        free(message.key);
        return NULL;
    }

    if (message_answer->opcode == MESSAGE_T__OPCODE__OP_GET + 1 &&
        message_answer->c_type == MESSAGE_T__C_TYPE__CT_VALUE)
    {
        void *data = malloc(message_answer->value.len);
        if (data == NULL)
        {
            message_t__free_unpacked(message_answer, NULL);
            free(message.key);
            return NULL;
        }
        memcpy(data, message_answer->value.data, message_answer->value.len);
        struct block_t *block = block_create(message_answer->value.len, data);
        struct block_t *block2 = block_duplicate(block);
        block_destroy(block);
        message_t__free_unpacked(message_answer, NULL);
        free(message.key);
        return block2;
    }
    message_t__free_unpacked(message_answer, NULL);
    free(message.key);
    return NULL;
}

int rtable_del(struct rtable_t *rtable, char *key)
{
    if (rtable == NULL || key == NULL)
    {
        return -1;
    }

    MessageT message;
    message_t__init(&message);
    message.opcode = MESSAGE_T__OPCODE__OP_DEL;
    message.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    message.key = strdup(key);
    if (message.key == NULL)
    {
        free(message.key);
        return -1;
    }

    MessageT *message_answer = network_send_receive(rtable, &message);
    if (message_answer == NULL)
    {
        free(message.key);
        return -1;
    }

    if (message_answer->opcode == MESSAGE_T__OPCODE__OP_DEL + 1 &&
        message_answer->c_type == MESSAGE_T__C_TYPE__CT_NONE)
    {
        message_t__free_unpacked(message_answer, NULL);
        free(message.key);
        return 0;
    }

    message_t__free_unpacked(message_answer, NULL);
    free(message.key);
    return -1;
}

int rtable_size(struct rtable_t *rtable)
{
    if (rtable == NULL)
    {
        return -1;
    }

    MessageT message;
    message_t__init(&message);
    message.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *message_answer = network_send_receive(rtable, &message);
    if (message_answer == NULL)
    {
        return -1;
    }

    if (message_answer->opcode == MESSAGE_T__OPCODE__OP_SIZE + 1 &&
        message_answer->c_type == MESSAGE_T__C_TYPE__CT_RESULT)
    {
        int result = message_answer->result;
        message_t__free_unpacked(message_answer, NULL);
        return result;
    }
    message_t__free_unpacked(message_answer, NULL);

    return -1;
}

char **rtable_get_keys(struct rtable_t *rtable)
{
    if (rtable == NULL)
    {
        return NULL;
    }

    MessageT message;
    message_t__init(&message);
    message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *message_answer = network_send_receive(rtable, &message);
    if (message_answer == NULL)
    {
        return NULL;
    }

    if (message_answer->opcode == MESSAGE_T__OPCODE__OP_GETKEYS + 1 &&
        message_answer->c_type == MESSAGE_T__C_TYPE__CT_KEYS)
    {
        char **keys = (char **) malloc(sizeof(char *) * (message_answer->n_keys + 1));
        if (keys == NULL)
        {
            message_t__free_unpacked(message_answer, NULL);
            return NULL;
        }
        for (int i = 0; i < message_answer->n_keys; i++)
        {   
            keys[i] = strdup(message_answer->keys[i]);
            if (keys[i] == NULL)
            {
                for (int j = 0; j < i; j++)
                {
                    free(keys[j]);
                }
                free(keys);
                message_t__free_unpacked(message_answer, NULL);
                return NULL;
            }
        }
        keys[message_answer->n_keys] = NULL;
        message_t__free_unpacked(message_answer, NULL);
        return keys;
    }
    message_t__free_unpacked(message_answer, NULL);

    return NULL;
}

void rtable_free_keys(char **keys)
{
    if (keys == NULL)
    {
        return;
    }

    for (int i = 0; keys[i] != NULL; i++)
    {
        free(keys[i]);
    }
    free(keys);
}

struct entry_t **rtable_get_table(struct rtable_t *rtable)
{
    if (rtable == NULL)
    {
        return NULL;
    }
   
    MessageT message;
    message_t__init(&message);
    message.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *message_answer = network_send_receive(rtable, &message);
    if (message_answer == NULL)
    {
        return NULL;
    }
    
    if (message_answer->opcode == MESSAGE_T__OPCODE__OP_GETTABLE + 1 &&
        message_answer->c_type == MESSAGE_T__C_TYPE__CT_TABLE)
    {
        struct entry_t ** entries = (struct entry_t **) malloc (sizeof (struct entry_t *) * (message_answer->n_entries + 1));
        if (entries == NULL)
        {
            message_t__free_unpacked(message_answer, NULL);
            return NULL;
        }
        for (int i = 0; i < message_answer->n_entries; i++)
        {
            entries[i] = (struct entry_t *) malloc (sizeof(struct entry_t));
            if (entries[i] == NULL)
            {
                for (int j = 0; j < i; j++) 
                {
                    free(entries[j]);
                }
                free(entries);
                message_t__free_unpacked(message_answer, NULL);
                return NULL;
            }
            entries[i]->key = strdup(message_answer->entries[i]->key);
            if (entries[i]->key == NULL)
            {
                for (int j = 0; j < i; j++)
                {
                    free(entries[j]->key);
                    free(entries[j]);
                }
                free(entries);
                message_t__free_unpacked(message_answer, NULL);
                return NULL;
            }

            void *data = malloc(message_answer->entries[i]->value.len);
            if (data == NULL)
            {
                for (int j = 0; j < i; j++)
                {
                    free(entries[j]->key);
                    free(entries[j]);
                }
                free(entries);
                message_t__free_unpacked(message_answer, NULL);
                return NULL;
            }
            memcpy(data, message_answer->entries[i]->value.data, message_answer->entries[i]->value.len);
            struct block_t *block1 = block_create(message_answer->entries[i]->value.len, data);
            entries[i]->value = block_duplicate(block1);
            block_destroy(block1);

        }
        entries[message_answer->n_entries] = NULL;
        message_t__free_unpacked(message_answer, NULL);
        return entries;
    }
    message_t__free_unpacked(message_answer, NULL);
    return NULL;
}

void rtable_free_entries(struct entry_t **entries)
{
    if (entries == NULL)
    {
        return;
    }

    for (int i = 0; entries[i] != NULL; i++)
    {
        free(entries[i]->key);
        block_destroy(entries[i]->value);
        free(entries[i]);
    }
    free(entries);
}

struct statistics_t *rtable_stats(struct rtable_t *rtable) 
{
    if (rtable == NULL)
    {
        return NULL;
    }
    
    MessageT message;
    message_t__init(&message);

    message.opcode = MESSAGE_T__OPCODE__OP_STATS;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *message_answer = network_send_receive(rtable, &message);
    if (message_answer == NULL)
    {
        return NULL;
    }

    if (message_answer->opcode == MESSAGE_T__OPCODE__OP_STATS + 1 &&
        message_answer->c_type == MESSAGE_T__C_TYPE__CT_STATS)
    {   

        struct statistics_t* stats = malloc(sizeof(struct statistics_t));
        stats->total_ops = (int) message_answer->stats->total_ops;
        stats->total_op_time = (int) message_answer->stats->total_op_time;
        stats->connected_clients = (int) message_answer->stats->connected_clients;

        message_t__free_unpacked(message_answer, NULL);
        
        return stats;
    }
    message_t__free_unpacked(message_answer, NULL);

    return NULL;
}
