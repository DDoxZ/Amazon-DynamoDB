// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#ifndef _BLOCK_H
#define _BLOCK_H /* Módulo BLOCK */

/* Estrutura que define um bloco de dados genérico.
 */
struct block_t {
	int datasize; /* Tamanho do bloco de dados */
	void *data;   /* Conteúdo arbitrário */
};

/* Função que cria um novo bloco de dados block_t e que inicializa 
 * os dados de acordo com os argumentos recebidos, sem necessidade de
 * reservar memória para os dados.	
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct block_t *block_create(int size, void *data); 

/* Função que duplica uma estrutura block_t, reservando a memória
 * necessária para a nova estrutura.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct block_t *block_duplicate(struct block_t *b);

/* Função que substitui o conteúdo de um bloco de dados block_t.
 * Deve assegurar que liberta o espaço ocupado pelo conteúdo antigo.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int block_replace(struct block_t *b, int new_size, void *new_data);

/* Função que elimina um bloco de dados, apontado pelo parâmetro b,
 * libertando toda a memória por ele ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int block_destroy(struct block_t *b);

#endif
