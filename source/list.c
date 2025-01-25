/** 
 * Distributed Systems
 * Project 1 - Group 26
 * 59790 - Francisco Catarino
 * 59822 - Pedro Simoes
 * 60447 - Diogo Lopes
*/

#include "../include/list.h"
#include "../include/list-private.h"
#include "../include/entry.h"

#include <stdlib.h>
#include <string.h>


struct list_t *list_create()
{
    struct list_t *l = (struct list_t *) malloc(sizeof(struct list_t));
    if (l == NULL)
    {
        return NULL;
    }
    l->head = NULL;
    l->size = 0;
    return l;
}

int list_add(struct list_t *l, struct entry_t *entry)
{
    if (l == NULL || entry == NULL)
    {
        return -1;
    }

    struct node_t *new, *cur, *prev;
    new = malloc(sizeof(struct node_t));
    new->entry = entry;
    new->next = NULL;

    if (l->size == 0)
    {
        l->head = new;
        l->size++;
        return 0;
    }

    prev = l->head;
    cur = prev->next;

    int entry_cmp = entry_compare(new->entry, prev->entry);
    switch(entry_cmp)
    {
        case 1:
            // gotta iterate throw the list to find the correct place to insert
            if(cur != NULL)
              entry_cmp = entry_compare(new->entry, cur->entry);

            while(cur != NULL && entry_cmp >= 0)
            {
                if(entry_cmp == 0)
                {
                    if(entry_destroy(cur->entry) == -1)
                    {
                      return -1;
                    }
                    cur->entry = new->entry;
                    free(new);
                    return 1;
                }
                prev = cur;
                cur = cur->next;
                if (cur != NULL)
                  entry_cmp = entry_compare(new->entry, cur->entry);
            }

            prev->next = new;
            new->next = cur;
            l->size++;
            return 0;

        case 0:
            // replace the entry, destroying the old one to avoid memory leaks
            if(entry_destroy(prev->entry) == -1)
            {
                return -1;
            }
            prev->entry = new->entry;
            free(new);
            return 1;

        case -1:
            // new entry will be the head
            new->next = prev;
            l->head = new;
            l->size++;
            return 0;

        case -2:
            free(new);
            return -1;
    }
    free(new);
    return -1;
}

int list_size(struct list_t *l)
{
    if (l == NULL)
    {
        return -1;
    }

    return l->size;
}

struct entry_t *list_get(struct list_t *l, char *key)
{
    if (l == NULL || key == NULL)
    {
        return NULL;
    }

    struct node_t *cur;
    cur = l->head;

    while(cur != NULL)
    {
        if(strcmp(cur->entry->key, key) == 0)
        {
            return cur->entry;
        }
        cur = cur->next;
    }

    return NULL;
}

char **list_get_keys(struct list_t *l)
{    
    if (l == NULL || l->head == NULL) {
        return NULL;
    }

    // +1 for the last element of the array to be NULL
    char **keys = (char **) malloc((l->size + 1) * sizeof(char*));
    if (keys == NULL)
    {
        return NULL;
    }

    struct node_t *cur;
    cur = l->head;

    for (int i = 0; i < l->size; i++)
    {
        // just to be sure no NULL keys flow through and cause a segfault
        if (cur->entry->key != NULL)
        {
            // +1 because the strlen doesn't count the '\0'
            keys[i] = (char *) malloc((strlen(cur->entry->key) + 1) * sizeof(char)); 
        }

        if (keys[i] == NULL || cur->entry->key == NULL)
        {
            // free here in case of error because we wont know how many keys were allocated before the error occured
            for (int j = 0; j < i; j++)
            {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
        strcpy(keys[i], cur->entry->key);
        cur = cur->next;
    }

    keys[l->size] = NULL;
    return keys;
}

int list_free_keys(char **keys)
{
    if (keys == NULL)
    {
        return -1;
    }

    // we made it end in NULL, so we can just iterate until we find it
    for (int i = 0; keys[i] != NULL; i++)
    {
        free(keys[i]);
    }

    free(keys);
    return 0;
}

int list_remove(struct list_t *l, char *key)
{
    if (l == NULL || key == NULL)
    {
        return -1;
    }

    struct node_t *cur, *prev;
    cur = l->head;
    prev = NULL;

    while(cur != NULL)
    {
        if(strcmp(cur->entry->key, key) == 0)
        {
            if (prev == NULL) // Removing 1st element
            {
                l->head = cur->next;
            }
            else // Removing any other element
            {
                prev->next = cur->next;
            }

            if(entry_destroy(cur->entry) == -1)
            {
                return -1;
            }

            free(cur);
            l->size--;
            return 0;
        }

        prev = cur;
        cur = cur->next;
    }

    return 1;
}

int list_destroy(struct list_t *l)
{
    if (l == NULL)
    {
        return -1;
    }

    char **keys = list_get_keys(l);
    if (keys == NULL)
    {
        free(l);
        return -1;
    }

    for (int i = 0; keys[i] != NULL; i++) {
        list_remove(l, keys[i]);
    }

    if(list_free_keys(keys) == -1)
    {
        free(l);
        return -1;
    }

    free(l);   
    return 0;
}
