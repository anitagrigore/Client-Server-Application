CFLAGS=-std=gnu99 -g -Iinclude
CC=gcc

server: server.o server_tcp.o server_udp.o linkedlist.o
	$(CC) $^ -o server -lm

server.o: server.c

linkedlist.o: linkedlist.c

server_tcp.o: server_tcp.c include/helper.h

server_udp.o: server_udp.c include/helper.h 

clean:
	rm *.o server

.PHONY: clean