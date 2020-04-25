CPPFLAGS=-std=c++14 -g -Iinclude
CC=g++

server: server.o server_tcp.o server_udp.o
	$(CC) $^ -o server -lm

server.o: server.cpp

server_tcp.o: server_tcp.cpp include/context_manager.h include/helper.h

server_udp.o: server_udp.cpp include/context_manager.h include/helper.h 

clean:
	rm *.o server

.PHONY: clean