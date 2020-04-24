#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "helper.h"
#include "linkedlist.h"
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

      return -1;
    }

    struct context ctx = {0};
    ctx.message_list = list_create();
    ctx.clients = list_create();
    ctx.pending = list_create();

    fd_set read_fds, tmp_fds;
    int fdmax;

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    int port = atoi(argv[1]);
    int sockfd_udp = server_udp(port);
    DIE(sockfd_udp == -1, "invalid socket");

    int sockfd_tcp = server_tcp(port);
    DIE(sockfd_tcp == -1, "invalid socket");

    FD_SET(sockfd_udp, &read_fds);
    FD_SET(sockfd_tcp, &read_fds);
    fdmax = sockfd_tcp;

    while(1) {
      tmp_fds = read_fds;

      int ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);

      for (int i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, &tmp_fds)) {
          if (i == sockfd_udp) {
            handle_udp_message(&ctx, sockfd_udp);
          } else if (i == sockfd_tcp) {
            handle_tcp_client(&ctx, sockfd_tcp);

            struct client_tcp* client_addr = ctx.pending->tail->data;
            FD_SET(client_addr->sockfd, &read_fds);

            if (client_addr->sockfd > fdmax) {
              fdmax = client_addr->sockfd;
            }

          } else if (i == STDIN_FILENO) {

          } else {
            printf("Socket: %d\n", i);

            if (handle_tcp_message(&ctx, i) <= 0)
            {
              close(i);
              FD_CLR(i, &read_fds);
            }
          }
        }
      }
    }


}
