/** 
 * Distributed Systems
 * Project 1 - Group 26
 * 59790 - Francisco Catarino
 * 59822 - Pedro Simoes
 * 60447 - Diogo Lopes
*/

#include "../include/serialization.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>


int keyArray_to_buffer(char **keys, char **keys_buf)
{
    if (keys == NULL || keys_buf == NULL)
    {
        return -1;
    }

    int buf_size = sizeof(int);
    int n_keys = 0;
    for (int i = 0; keys[i] != NULL; i++)
    {
        buf_size += strlen(keys[i]) + 1;
        n_keys++;
    }

    *keys_buf = (char*) malloc(buf_size);
    if (*keys_buf == NULL)
    {
        return -1;
    }

    int size = htonl(n_keys);
    memcpy(*keys_buf, &size, sizeof(int));

    for (int i = 0; i < n_keys; i++)
    {
        int key_lengt = strlen(keys[i]) + 1;
        memcpy(*keys_buf + sizeof(int) + i * key_lengt, keys[i], key_lengt);
    }
    
    return buf_size;
}

char** buffer_to_keyArray(char *keys_buf)
{
    if (keys_buf == NULL)
    {
        return NULL;
    }

    int nkeys;
    memcpy(&nkeys, keys_buf, sizeof(int));
    int size = (int) ntohl((uint32_t) nkeys); 
   
    char **keys = (char **) malloc((size + 1) * sizeof(char*));
    if (keys == NULL)
    {
        return NULL;
    }

    int index = sizeof(int);
    for (int i = 0; i < size; i++)
    {
        keys[i] = (char *) malloc(strlen(keys_buf + index) + 1);
        if (keys[i] == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
        strcpy(keys[i], keys_buf + index);
        index += strlen(keys[i]) + 1;
    }

    keys[size] = NULL;
    return keys;  
}