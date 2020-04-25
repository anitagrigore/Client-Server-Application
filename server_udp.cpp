#include "helper.h"
#include "server_udp.h"
#include "udp_utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int Server_UDP::start()
{
  int sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd_udp == -1) {
    perror("socket");
    return -1;
  }

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd_udp, (sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
    perror("bind");
    return -1;
  }
  
  server_sock = sockfd_udp;

  return sockfd_udp;
}

int Server_UDP::handle_message()
{
  UDPMessage p{};
  p.source.addrlen = sizeof(p.source.addr);

  ssize_t n_recv = recvfrom(server_sock, &p.message, sizeof(p.message), 0,
        (struct sockaddr*) &p.source.addr, &p.source.addrlen);
  if (n_recv == -1) {
    perror("recvfrom");
    return -1;
  }

  ctx.pending_messages.push(p);

  return 0;
}
