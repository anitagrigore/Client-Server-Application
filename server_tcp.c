#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 1024

int server_tcp(uint16_t port_nr)
{
  int sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd_tcp < 0) {
    perror("socket_tcp");
    return -1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(port_nr);
  server.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd_tcp, (struct sockaddr *) &server, sizeof(struct sockaddr)) < 0) {
    perror("bind");
    return -1;
  }

  if (listen(sockfd_tcp, BACKLOG) < 0) {
    perror("listen");
    return -1;
  }

  fd_set read_fds, tmp_fds;
  int fdmax;
  FD_ZERO(&read_fds);
  FD_ZERO(&tmp_fds);
  FD_SET(sockfd_tcp, &read_fds);
  fdmax = sockfd_tcp;

}
