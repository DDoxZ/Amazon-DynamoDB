// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H /* Módulo TABLE_PRIVATE */

#include "list.h"

struct table_t {
	struct list_t **lists;
	int size;
};

/* Função que calcula o índice da lista a partir da chave
 */
int hash_code(char *key, int n);

#endif
