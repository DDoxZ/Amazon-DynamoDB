// Distributed Systems 
// Project 4 - Group 26
// 59790 - Francisco Catarino
// 59822 - Pedro Simoes
// 60447 - Diogo Lopes

#include "../include/inet-private.h"

#include <errno.h>

int write_all(int sock, char *buf, int len) {
    int bufsize = len;
    fflush(stdout);
    while(len>0) {
        int res = write(sock, buf, len);
        if(res<0) {
            if(errno==EINTR) continue;
            perror("write failed:");
            return res;
        }
        if(res==0) return res;
        buf += res;
        len -= res;
    }
    return bufsize;
}

int read_all(int sock, char *buf, int len) {
    int bufsize = len;
    while(len>0) {
        int res = read(sock, buf, len);
        if(res<0) {
            if(errno==EINTR) continue;
            perror("read failed:");
            return res;
        }
        if(res==0) return res;
        buf += res;
        len -= res;
    }
    return bufsize;
}
