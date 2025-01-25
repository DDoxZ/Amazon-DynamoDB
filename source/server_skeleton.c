// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#include "block.h"
#include "entry.h"
#include "table.h"
#include "stats.h"
#include "server_skeleton-private.h"
#include "htmessages.pb-c.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>


struct statistics_t* stats;

struct table_t *server_skeleton_init(int n_lists)
{
    stats = (struct statistics_t *)malloc(sizeof(struct statistics_t));
    if (stats == NULL)
    {
        return NULL;
    }
    stats->total_ops = 0;
    stats->total_op_time = 0;
    stats->connected_clients = 0;

    return table_create(n_lists);
}

int server_skeleton_destroy(struct table_t *table)
{
    free(stats);
    return table_destroy(table);
}


int messageError(MessageT *msg)
{
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    return -1;
}

int msgPut(MessageT *msg, struct table_t *table)
{   
    //PODE NAO CORRER -> APENAS ANDAR OU COXEAR
    //ver com atenção o value que é um ProtobufCBinaryData

    struct block_t *block = block_create(msg->entry->value.len, msg->entry->value.data);
    
    if (block == NULL)
    {
        return messageError(msg);
    }

    if (table_put(table, msg->entry->key, block) == -1)
    {
        return messageError(msg);
    }

    block_destroy(block);
    free(msg->entry);
    
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    return 0;
}

int msgGet(MessageT *msg, struct table_t *table)
{
    struct block_t *block = table_get(table, msg->key);
    if (block == NULL)
    {
        return messageError(msg);
    }

    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;

    void *data = malloc(block->datasize);
    if (data == NULL)
    {
        return messageError(msg);
    }
    memcpy(data, block->data, block->datasize);
    msg->value.data = data;
    msg->value.len = block->datasize;
    block_destroy(block);

    return 0;
}

int msgDel(MessageT *msg, struct table_t *table)
{
    if (table_remove(table, msg->key) == -1)
    {
        return messageError(msg);
    } 

    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    return 0;
}

int msgSize(MessageT *msg, struct table_t *table)
{   
    int size = table_size(table);
    if (size == -1)
    {
        return messageError(msg);
    } 

    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg->result = size;

    return 0;
}

int msgGetKeys(MessageT *msg, struct table_t *table)
{   
    //Libertar a memória 
    char ** keys = table_get_keys(table);

    if (keys == NULL)
    {
        return messageError(msg);
    }

    int number_of_keys = table_size(table);
    if (number_of_keys == -1)
    {
        return messageError(msg);
    }
    //dar free a msg
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
    msg->keys = keys;
    msg->n_keys = number_of_keys;
    return 0;
} 

int msgGetTable(MessageT *msg, struct table_t *table)
{
    char ** keys = table_get_keys(table);
    if (keys == NULL)
    {
        return messageError(msg);
    }

    int length = 0;
    while (keys[length] != NULL) 
    {
        length++;
    }

    EntryT **entries = malloc(sizeof(EntryT *) * length);
    if (entries == NULL) 
    {
        for (int i = 0; keys[i] != NULL; i++) 
        {
            free(keys[i]);
        }
        free(keys);
        return messageError(msg);
    }

    for (int i = 0; i < length; i++)
    {
        struct block_t *block = table_get(table, keys[i]);
        if (block == NULL)
        {
            for (int j = 0; j < i; j++) 
            {
                free(entries[j]->value.data);
                free(entries[j]);
            }
            free(entries);
            for (int j = 0; keys[j] != NULL; j++) 
            {
                free(keys[j]);
            }
            free(keys);
            return messageError(msg);
        }

        entries[i] = malloc(sizeof(EntryT));
        if (entries[i] == NULL)
        {
            for (int j = 0; j < i; j++) 
            {
                free(entries[j]->value.data);
                free(entries[j]);
            }
            free(entries);
            for (int j = 0; keys[j] != NULL; j++) 
            {
                free(keys[j]);
            }
            free(keys);
            return messageError(msg);
        }

        entry_t__init(entries[i]);
        entries[i]->key = keys[i];

        void *data = malloc(block->datasize);
        if (data == NULL)
        {
            for (int j = 0; j <= i; j++) 
            {
                free(entries[j]->value.data);
                free(entries[j]);
            }
            free(entries);
            for (int j = 0; keys[j] != NULL; j++) 
            {
                free(keys[j]);
            }
            free(keys);
            return messageError(msg);
        }
        memcpy(data, block->data, block->datasize);
        entries[i]->value.data = data;
        entries[i]->value.len = block->datasize;
        block_destroy(block);
    }

    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
    msg->entries = entries;
    msg->n_entries = length;

    free(keys);
    return 0;
}

int msgGetStats(MessageT *msg, struct table_t *table) {

    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_STATS + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;


    StatisticsT *statistics = malloc(sizeof(StatisticsT));
    statistics_t__init(statistics);

    statistics->total_ops = (int32_t) stats->total_ops;
    statistics->total_op_time = (int32_t) stats->total_op_time;
    statistics->connected_clients = (int32_t) stats->connected_clients;

    msg->stats = statistics;

    return 0;
}

int invoke(MessageT *msg, struct table_t *table)
{
    if (msg == NULL || table == NULL)
    {
        return -1;
    }

    switch (msg->opcode)
    {   
        /*MESSAGE_T__OPCODE__OP_BAD = 0,
        MESSAGE_T__OPCODE__OP_PUT = 10,
        MESSAGE_T__OPCODE__OP_GET = 20,
        MESSAGE_T__OPCODE__OP_DEL = 30,
        MESSAGE_T__OPCODE__OP_SIZE = 40,
        MESSAGE_T__OPCODE__OP_GETKEYS = 50,
        MESSAGE_T__OPCODE__OP_GETTABLE = 60,
        MESSAGE_T__OPCODE__OP_ERROR = 99
        */ 

       /*
        MESSAGE_T__C_TYPE__CT_BAD = 0,
        MESSAGE_T__C_TYPE__CT_ENTRY = 10,
        MESSAGE_T__C_TYPE__CT_KEY = 20,
        MESSAGE_T__C_TYPE__CT_VALUE = 30,
        MESSAGE_T__C_TYPE__CT_RESULT = 40,
        MESSAGE_T__C_TYPE__CT_KEYS = 50,
        MESSAGE_T__C_TYPE__CT_TABLE = 60,
        MESSAGE_T__C_TYPE__CT_NONE = 70
       */
        case MESSAGE_T__OPCODE__OP_BAD :
            return messageError(msg);

        case MESSAGE_T__OPCODE__OP_PUT :
            if (msg->c_type == MESSAGE_T__C_TYPE__CT_ENTRY)
                return msgPut(msg,table);
            return messageError(msg);

        case MESSAGE_T__OPCODE__OP_GET :
            if (msg->c_type == MESSAGE_T__C_TYPE__CT_KEY)
                return msgGet(msg,table);
            return messageError(msg);

        case MESSAGE_T__OPCODE__OP_DEL :
            if (msg->c_type == MESSAGE_T__C_TYPE__CT_KEY)
                return msgDel(msg,table);
            return messageError(msg);

        case MESSAGE_T__OPCODE__OP_SIZE :
            if (msg->c_type == MESSAGE_T__C_TYPE__CT_NONE)
                return msgSize(msg,table);
            return messageError(msg);

        case MESSAGE_T__OPCODE__OP_GETKEYS :
            if (msg->c_type == MESSAGE_T__C_TYPE__CT_NONE)
                return msgGetKeys(msg,table);
            return messageError(msg);

        case MESSAGE_T__OPCODE__OP_GETTABLE :
            if (msg->c_type == MESSAGE_T__C_TYPE__CT_NONE)
                return msgGetTable(msg,table);
            return messageError(msg);

        case MESSAGE_T__OPCODE__OP_STATS :
            if (msg->c_type == MESSAGE_T__C_TYPE__CT_NONE)
                return msgGetStats(msg,table);
            return messageError(msg);

        case MESSAGE_T__OPCODE__OP_ERROR : 
            return messageError(msg);

        default:
            return messageError(msg);

    }

}

void increment_clients() 
{
    stats->connected_clients++;
}

void decrement_clients() 
{
    stats->connected_clients--;
}

void increment_time(int time) 
{
    stats->total_op_time += time;
}

void increment_total_ops() 
{
    stats->total_ops++;
}

void decrement_total_ops() 
{
    stats->total_ops--;
}
