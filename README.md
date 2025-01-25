# Amazon-DynamoDB

![Project IMG](DB.png)

Data Base similar to Amazon DynamoDB, made in C

**This Project was made in collaboration with two other classmates:**<br>
Pedro Simoes <br>
Francisco Catarino <br>

## About
This project involves the development of a secure and reliable key-value database inspired by Amazon DynamoDB, designed to support multiple web services with high availability and fault tolerance. The system processes concurrent client requests efficiently using multithreading, ensuring robust concurrency management and accurate shared data access. It incorporates fault tolerance through server state replication, leveraging the Chain Replication model and Apache ZooKeeper to maintain data consistency and reliability. The database guarantees durability, never losing data or experiencing downtime, making it a dependable solution for critical applications.
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
