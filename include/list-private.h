// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H /* Módulo LIST_PRIVATE */

#include "entry.h"

struct node_t {
	struct entry_t *entry;
	struct node_t  *next; 
};

struct list_t {
	int size;
	struct node_t *head;
};


#endif
