/** 
 * Distributed Systems
 * Project 1 - Group 26
 * 59790 - Francisco Catarino
 * 59822 - Pedro Simoes
 * 60447 - Diogo Lopes
*/

#include "../include/block.h"

#include <stdlib.h>
#include <string.h>


struct block_t *block_create(int size, void *data) 
{
    if (size <= 0 || data == NULL)
    {
        return NULL;
    }

    struct block_t* block = (struct block_t*) malloc (sizeof(struct block_t));
    if (block == NULL)
    {
        return NULL;
    }
    
    block->data = data;
    block->datasize = size;

    return block;
}

struct block_t *block_duplicate(struct block_t *b) 
{
    if (b == NULL || b->data == NULL || b->datasize <= 0) 
    {
        return NULL;
    }

    void *block_data = (void*) malloc(b->datasize);
    if (block_data == NULL)
    {
        return NULL;
    }

    memcpy(block_data, b->data, b->datasize);
    struct block_t *block_duplicate = block_create(b->datasize, block_data);
    
    return block_duplicate;
}

int block_replace(struct block_t *b, int new_size, void *new_data) 
{
    if (b == NULL || new_data == NULL || new_size <= 0)
    {
        return -1;
    }

    b->datasize = new_size;
    b->data = new_data;
    
    return 0;
}

int block_destroy(struct block_t *b)
{
    if (b == NULL)
    {
        return -1;
    }
    
    if(b->data != NULL)
    {
        free(b->data);
    }

    free(b);
    return 0;
}
