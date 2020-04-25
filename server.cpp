#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

#include "helper.h"
#include "context_manager.h"
#include "server_udp.h"
#include "server_tcp.h"

int main(int argc, char** argv)
{
  //fd-uri
  //apelez functia pt sockfd_udp
  //apelez functia pr socket_tcp
  //fd_read += sockfd_udp + sockfd_tcp +stdin
  //select in while(1) => verific nr socket (daca e de udp sau tcp)
  //    =>udp -> handle_message_udp
  //    =>tcp -> handle_conections (accept + altele)
  //    =>stdin -> handle_keyboard_input -> daca exit, break

  if (argc < 2) {
    fprintf(stderr, "usage: server port_number\n");
    return -1;
  }

  ContextManager ctx{};

  fd_set read_fds, tmp_fds;
  int fdmax;

  FD_ZERO(&read_fds);
  FD_ZERO(&tmp_fds);

  int port = atoi(argv[1]);
  
  Server_UDP udp_server{ctx, port};
  udp_server.start();
  DIE(udp_server.get_fd() == -1, "udp server failed to start");

  Server_TCP tcp_server{ctx, port};
  tcp_server.start();

  DIE(tcp_server.get_fd() == -1, "tcp server failed to start");

  FD_SET(udp_server.get_fd(), &read_fds);
  FD_SET(tcp_server.get_fd(), &read_fds);
  
  fdmax = tcp_server.get_fd();

  while(1) {
    tmp_fds = read_fds;

    int ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);

    for (int i = 0; i <= fdmax; i++) {
      if (!FD_ISSET(i, &tmp_fds)) {
        continue;
      }
      
      if (i == udp_server.get_fd()) {
        udp_server.handle_message();
      } else if (i == tcp_server.get_fd()) {
        int clientfd = tcp_server.handle_client();

        FD_SET(clientfd, &read_fds);
        fdmax = std::max(clientfd, fdmax);
      } else if (i == STDIN_FILENO) {

      } else {
        printf("Socket ready: %d\n", i);

        if (tcp_server.handle_message(i) <= 0) {
          close(i);
          FD_CLR(i, &read_fds);
        }
      }
    }
  }
}
