# Distributed Systems 
# Project 2 - Group 26
# 59790 - Francisco Catarino
# 59822 - Pedro Simoes
# 60447 - Diogo Lopes

OBJ_dir = object
SRC_dir = source
BIN_dir = binary
INC_dir = include
LIB_dir = lib

CLIENT_OBJECTS = $(OBJ_dir)/client_hashtable.o \
                 $(OBJ_dir)/client_network.o \
                 $(OBJ_dir)/client_stub.o \
                 $(OBJ_dir)/htmessages.pb-c.o \
								 $(OBJ_dir)/message.o

SERVER_OBJECTS = $(OBJ_dir)/server_hashtable.o \
                 $(OBJ_dir)/server_network.o \
                 $(OBJ_dir)/server_skeleton.o \
                 $(OBJ_dir)/htmessages.pb-c.o \
								 $(OBJ_dir)/client_stub.o \
								 $(OBJ_dir)/client_network.o \
								 $(OBJ_dir)/message.o

LIB_OBJECTS = $(OBJ_dir)/block.o \
              $(OBJ_dir)/entry.o \
              $(OBJ_dir)/list.o \
              $(OBJ_dir)/table.o

LIBRARY = libtable.a
EXEC_CLIENT = client_hashtable
EXEC_SERVER = server_hashtable

# Compiler
CC = gcc
PROTOC = protoc-c --c_out=.

# Flags
#flag do zookeeper
CFLAGS = -Wall -I $(INC_dir) -I /usr/include/zookeeper -g
LIBFLAGS = -rcs
LDFLAGS = -lprotobuf-c -lzookeeper_mt


all: protobuffs libtable client_hashtable server_hashtable

libtable: $(LIB_OBJECTS)
	@mkdir -p $(LIB_dir)
	ar $(LIBFLAGS) $(addprefix $(LIB_dir)/,$(LIBRARY)) $(LIB_OBJECTS)

client_hashtable: libtable $(CLIENT_OBJECTS)
	@mkdir -p $(BIN_dir)
	$(CC) $(CFLAGS) $(CLIENT_OBJECTS) -o $(BIN_dir)/$(EXEC_CLIENT) -L $(LIB_dir) -ltable $(LDFLAGS)

server_hashtable: $(SERVER_OBJECTS) $(LIB_dir)/libtable.a
	@mkdir -p $(BIN_dir)
	$(CC) $(CFLAGS) $(SERVER_OBJECTS) -o $(BIN_dir)/$(EXEC_SERVER) -L $(LIB_dir) -ltable $(LDFLAGS)

# Regra para compilar arquivos .c em .o
$(OBJ_dir)/%.o: $(SRC_dir)/%.c
	@mkdir -p $(OBJ_dir)
	$(CC) $(CFLAGS) -c $< -o $@

protobuffs: $(SRC_dir)/htmessages.proto
	$(PROTOC) $(SRC_dir)/htmessages.proto
	sed -i 's|#include "source/htmessages.pb-c.h"|#include "htmessages.pb-c.h"|' $(SRC_dir)/htmessages.pb-c.c
	mv $(SRC_dir)/htmessages.pb-c.h $(INC_dir)

# Limpeza dos arquivos compilados
clean:
	rm -rf $(BIN_dir)/* $(LIB_dir)/libtable.a
	rm -rf $(OBJ_dir)/client_hashtable.o  $(OBJ_dir)/client_network.o $(OBJ_dir)/client_stub.o \
			$(OBJ_dir)/server_hashtable.o $(OBJ_dir)/server_network.o $(OBJ_dir)/server_skeleton.o \
			$(OBJ_dir)/htmessages.o $(OBJ_dir)/message.o


		