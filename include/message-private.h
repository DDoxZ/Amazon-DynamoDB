// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

int write_all(int sock, void *buf, int len);

int read_all(int sock, void *buf, int len);

#endif