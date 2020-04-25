#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#include "client_tcp.h"
#include "helper.h"

int main(int argc, char** argv)
{
  if (argc < 4) {
    fprintf(stderr, "usage: client client_id ip port_number\n");
    return -1;
  }
  
  int port = atoi(argv[3]);
  if (port == 0) {
    fprintf(stderr, "invalid port\n");
  }
  
  Client_TCP tcp_client{argv[2], port};
  tcp_client.start();
  DIE(tcp_client.get_fd() == -1, "tcp client failed to start");
  
  // say hello
  tcp_client.say_hello(argv[1]);
  
  fd_set read_fds, temp_fds;

  FD_ZERO(&read_fds);
  FD_ZERO(&temp_fds);
  
  FD_SET(STDIN_FILENO, &read_fds);
  FD_SET(tcp_client.get_fd(), &read_fds);
  int fdmax = tcp_client.get_fd();
  
  bool finished = false;
  while(!finished) {
    temp_fds = read_fds;
    
    int ret = select(fdmax + 1, &temp_fds, NULL, NULL, NULL);
    
    for (int i = 0; i <= fdmax; i++) {
      if (!FD_ISSET(i, &temp_fds)) {
        continue;
      }
      
      if (i == STDIN_FILENO) {
        if (tcp_client.handle_command() == 0) {
          finished = true;
          break;
        }

        continue;
      }
      
      if (i == tcp_client.get_fd()) {
        if (tcp_client.handle_server_message() == 0) {
          finished = true;
          break;
        }
        
        continue;
      }
    }
  }
}