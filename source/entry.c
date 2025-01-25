/** 
 * Distributed Systems
 * Project 1 - Group 26
 * 59790 - Francisco Catarino
 * 59822 - Pedro Simoes
 * 60447 - Diogo Lopes
*/

#include "../include/block.h"
#include "../include/entry.h"

#include <stdlib.h>
#include <string.h>


struct entry_t *entry_create(char *key, struct block_t *value)
{
    if (key == NULL || value == NULL)
    {
        return NULL;
    }

    struct entry_t* entry = (struct entry_t*) malloc (sizeof(struct entry_t));
    
    if (entry == NULL)
    {
        return NULL;
    }
    
    entry->key = key;
    entry->value = value;

    return entry;
}

int entry_compare(struct entry_t *e1, struct entry_t *e2)
{
    if (e1 ==NULL || e2 == NULL)
    {
        return -2;
    }

    int temp = strcmp(e1->key, e2->key);
    
    if (temp < 0) return -1;
    if (temp > 0) return 1;
    return 0;

}

struct entry_t *entry_duplicate(struct entry_t *e)
{
    if (e == NULL || e->key == NULL || e->value == NULL)
    {
        return NULL;
    }

    struct entry_t* entry = (struct entry_t*) malloc(sizeof(struct entry_t));
    if (entry == NULL)
    {
        return NULL;
    }

    entry->value = block_duplicate(e->value);
    if (entry->value == NULL)
    {
        free(entry);
        return NULL;
    }

    entry->key = malloc(strlen(e->key) + 1);
    if (entry->key == NULL)
    {
        free(entry->value);
        free(entry);
        return NULL;
    }

    memcpy(entry->key, e->key, strlen(e->key) + 1);

    return entry;
}

int entry_replace(struct entry_t *e, char *new_key, struct block_t *new_value)
{
    if (e == NULL || new_key == NULL || new_value == NULL)
    {
        return -1;
    }

    block_destroy(e->value);
    free(e->key);

    e->key = new_key;
    e->value = new_value;
    return 0;
    
}

int entry_destroy(struct entry_t *e)
{
    if (e == NULL || e->key == NULL || e->value == NULL) 
    {
        return -1;
    }
    
    free(e->key);
    block_destroy(e->value);
    free(e);
    return 0;
}