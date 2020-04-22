CFLAGS=-std=c99 -g -Iinclude

server: server.c server_tcp.o server_udp.o linkedlist.o

linkedlist.o: linkedlist.c

server_tcp.o: server_tcp.c include/helper.h linkedlist.o

server_udp.o: server_udp.c include/helper.h linkedlist.o

clean:
	rm *.o server
