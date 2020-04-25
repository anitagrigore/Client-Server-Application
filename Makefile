CPPFLAGS=-std=c++14 -g -Iinclude
CC=g++

all: server client

server: server.o server_tcp.o server_udp.o
	$(CC) $^ -o server -lm
	
client: client.o client_tcp.o
	$(CC) $^ -o client -lm

server.o: server.cpp

server_tcp.o: server_tcp.cpp include/context_manager.h include/helper.h

server_udp.o: server_udp.cpp include/context_manager.h include/helper.h 

client.o: client.cpp

client_tcp.o: client_tcp.cpp include/helper.h include/client_tcp.h

clean:
	rm *.o server client

.PHONY: clean all
