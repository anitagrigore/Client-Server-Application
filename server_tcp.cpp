#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <algorithm>

#include "server_tcp.h"
#include "tcp_utils.h"

#define BACKLOG 1024

int Server_TCP::start()
{
  int sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd_tcp < 0) {
    perror("socket_tcp");
    return -1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd_tcp, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("bind");
    return -1;
  }

  if (listen(sockfd_tcp, BACKLOG) < 0) {
    perror("listen");
    return -1;
  }
  
  server_sock = sockfd_tcp;

  return sockfd_tcp;
}

int Server_TCP::handle_client()
{
  TCPClient pending_conn{};
  pending_conn.addrlen = sizeof(struct sockaddr_storage);

  int clientfd = accept(server_sock, (struct sockaddr*) &pending_conn.addr, &pending_conn.addrlen);
  if (clientfd < 0) {
    perror("accept");
    return -1;
  }

  pending_conn.sockfd = clientfd;
  ctx.pending_conns.push_back(pending_conn);

  return clientfd;
}

int Server_TCP::handle_hello_message(int clientfd, const std::string &id)
{
  auto pending_conn = ctx.pending_conns.end();

  for (auto it = ctx.pending_conns.begin(); it != ctx.pending_conns.end(); it++) {
    if (it->sockfd == clientfd) {
      pending_conn = it;
      break;
    }
  }

  if (pending_conn == ctx.pending_conns.end()) {
    std::cerr << "Unexpected hello message.\n";
    return -1;
  }

  for (auto &c : ctx.clients) {
    if (c.id == id) {
      if (c.sockfd == -1) {
        c.sockfd = pending_conn->sockfd;
        c.addr = pending_conn->addr;
        c.addrlen = pending_conn->addrlen;

        std::cout << "Client " << id << " reconnected.\n";

        ctx.pending_conns.erase(pending_conn);
        return 0;
      } else {
        std::cerr << "Username is already in use.\n";
        ctx.pending_conns.erase(pending_conn);
        return -1;
      }
    }
  }
  
  TCPClient client = *pending_conn;
  client.id = id;

  ctx.pending_conns.erase(pending_conn); // Remove the pending connection
  ctx.clients.push_back(client);  // Add the new client to known clients list.

  std::cout << "Client " << id << " connected.\n";

  return 0;
}

int Server_TCP::handle_message(int clientfd)
{
  char buf[MAX_PAYLOAD_LEN] = {0};
  int n_read = read(clientfd, buf, sizeof(buf));
  if (n_read == -1) {
    perror("handle_message: read");
    return -1;
  }

  if (n_read == 0) {
    disconnect_client(clientfd);
    return 0;
  }

  printf("tcp_msg_buffer = ");
  for (int i = 0; i < n_read; i++)
  {
    printf("%02x ", buf[i] & 0xff);
  }
  printf("\n");

  TCPMessageHeader* hdr = (TCPMessageHeader*) buf;
  char* payload = buf + sizeof(TCPMessageHeader);

  switch (hdr->type) {
    case HELLO:
    {
      char id[MAX_ID_LEN + 1]{};
      memcpy(id, payload, sizeof(std::min<size_t>(MAX_ID_LEN, ntohs(hdr->size))));
      
      handle_hello_message(clientfd, std::string(id));
      break;
    }
  }

  return n_read;
}

void Server_TCP::disconnect_client(int clientfd)
{
  std::cout << "TODO: Disconnecting client " << clientfd << "\n";
}
