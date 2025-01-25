# Amazon-DynamoDB
Data Base similar to Amazon DynamoDB in C

**This Project was made in collaboration with two other classmates:**<br>
Pedro Simoes
Francisco Catarino

## About
Data Base service based in key-value format similar to the one used by Amazon DynamoDB to support multiple web services. 
This Data Base stores information and can tolerate failiures without losing data or ever going down.
This project was developed as part of the Distributed Systems subject in university, with the help of two colleagues.

## What we learned
  - Client-Server Architectures
  - Apache Zookeeper
  - TCP and UDP Sockets
  - Protocol Buffers
  - Memory Management and I/O
  - Debugging Tools
  - Pthreads

---
## Directories
This project is divided into these directories:
- ```include```: to store the .h files;
- ```source```: to store the .c files;
- ```lib```: to store libraries;
- ```object```: to store object files;
- ```binary```: to store executable files.

---
## Compilation
```bash
make clean
```
```bash
make
```

---
## Execution

```bash
format: client_hashtable <server>:<port>
./client_hashtable localhost:2181
```

```bash
format: server_hashtable <zookeeper ip:port> <port> <n_lists>
./server_hashtable localhost:2181 12345 3
```

---
## Operations
1. ```put <key> <value>```
2. ```get <key>```
3. ```del <key>```
4. ```size```
5. ```getkeys```
6. ```gettable```
7. ```quit```
