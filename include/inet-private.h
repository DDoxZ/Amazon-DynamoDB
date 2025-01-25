// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//tamanho máximo da mensagem enviada pelo cliente
#define MAX_MSG 2048
