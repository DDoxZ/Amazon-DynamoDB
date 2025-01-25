/** 
 * Distributed Systems
 * Project 1 - Group 26
 * 59790 - Francisco Catarino
 * 59822 - Pedro Simoes
 * 60447 - Diogo Lopes
*/

#include "../include/table.h"
#include "../include/table-private.h"
#include "../include/list.h"
#include "../include/list-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct table_t *table_create(int n) {
    if (n <= 0) return NULL;

    struct table_t* table = (struct table_t*) malloc(sizeof(struct table_t));
    if (table == NULL) return NULL; 

    table->size = n;
    table->lists = (struct list_t **) malloc(sizeof(struct list_t) * n);

    if (table->lists == NULL)
    {
        free(table);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        table->lists[i] = list_create();
    }
    
    return table;
}

int hash_function(char* key, int n_lists) {
    if (key == NULL || n_lists <= 0) return -1;

    int sum = 0;
    char* c = key;

    while (*c != '\0') {
        sum += (int) *c;
        c++;
    }

    return sum % n_lists; 
}

int table_put(struct table_t *t, char *key, struct block_t *value) {
    if (t == NULL || key == NULL || value == NULL) return -1;

    int exists;
    int codigo;

    char* key_copy = (char *) malloc(strlen(key) + 1);
    if (key_copy == NULL) return -1;

    strcpy(key_copy, key);
    int hash = hash_function(key,t->size);
    struct block_t* block_copy = block_duplicate(value);
    
    struct entry_t* current_entry = list_get(t->lists[hash], key_copy);
    exists = current_entry != NULL;


    if (exists) {
        codigo = entry_replace(current_entry, key_copy, block_copy);
    } 
    else {
        struct entry_t* entry = entry_create(key_copy, block_copy);
        codigo = list_add(t->lists[hash], entry);
    }
    
    return codigo;
}

struct block_t *table_get(struct table_t *t, char *key) {
    if (t == NULL || key == NULL) return NULL;

    int hash = hash_function(key, t->size);
    struct entry_t* e = list_get(t->lists[hash], key); 

    return e == NULL ? NULL : block_duplicate(e->value);
}
    
int table_size(struct table_t *t) {
    if (t == NULL) return -1;

    int size = 0;

    for (int i = 0; i < t->size; i++) { //caso o tamanho da lista de erro age como se estivesse vazia

        int l_size = list_size(t->lists[i]);
        if (l_size == -1) return -1;
        size += l_size >= 0 ? l_size : 0;
    }

    return size;
}

char **table_get_keys(struct table_t *t) {
    if (t == NULL) return NULL;

    int key_counter = 0;
    char** keys = (char **)malloc((table_size(t) + 1) * sizeof(char *));
    if (keys == NULL) return NULL;

    for (int i = 0; i < t->size; i++) {

        char** l = list_get_keys(t->lists[i]);
        
        for (int j = 0; j < list_size(t->lists[i]); j++) {
            keys[key_counter] = malloc((strlen(l[j]) + 1) * sizeof(char));
            if (keys[key_counter] == NULL) 
            {
              for (int k = 0; k < key_counter; k++) {
                  free(keys[k]);
              }
              free(keys);
              list_free_keys(l);
              return NULL;
            }
            strcpy(keys[key_counter], l[j]);
            key_counter++;
        }
        list_free_keys(l);
    }
    keys[key_counter] = NULL;
    return keys;
}

int table_free_keys(char **keys) {
    if (keys == NULL) {
        return -1; 
    }

    for (int i = 0; keys[i] != NULL; i++) {
        free(keys[i]);
    }
    free(keys);

    return 0; 
}

int table_remove(struct table_t *t, char *key) {
    if (t == NULL || key == NULL) return -1;

    int hash = hash_function(key, t->size);
    return list_remove(t->lists[hash], key);
}

int table_destroy(struct table_t *t) {
    if (t == NULL) return -1;

    char** keys = table_get_keys(t);

    if (keys == NULL) return -1;
    
    int i = 0;
    int code = 0;
    int temp = 0;

    while (keys[i] != NULL) {
        code = table_remove(t, keys[i]);
        i++;
    }


    for (int i = 0; i < t->size; i++) {
        temp = list_destroy(t->lists[i]);
        code = temp == -1 ? -1 : code;
    }

    free(t->lists);
    free(t);
    table_free_keys(keys);


    return code;
}
