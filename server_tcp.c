#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server_tcp.h"

#define BACKLOG 1024

int server_tcp(uint16_t port_nr)
{
  int sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd_tcp < 0) {
    perror("socket_tcp");
    return -1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_nr);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd_tcp, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("bind");
    return -1;
  }

  if (listen(sockfd_tcp, BACKLOG) < 0) {
    perror("listen");
    return -1;
  }

  return sockfd_tcp;
}

int handle_tcp_client(struct context* ctx, int sockfd_tcp)
{
  struct client_tcp *client = malloc(sizeof(struct client_tcp*));
  client->addrlen = sizeof(struct sockaddr_storage);

  int sockfd_cli = accept(sockfd_tcp, (struct sockaddr*) &client->addr, &client->addrlen);
  if (sockfd_cli < 0) {
    // eroare
    return -1;
  }

  client->sockfd = sockfd_cli;
  list_insert(ctx->pending, client, NULL);

  return 0;
}

int handle_hello_message(struct context* ctx, int sockfd, char *id)
{
  struct client_tcp* client = NULL;
  struct list_node* curr;
  for (curr = ctx->pending->head; curr != NULL; curr = curr->next) {
    struct client_tcp* c = curr->data;
    if (c->sockfd == sockfd) {
      client = c;
      break;
    }
  }

  if (client == NULL) {
    printf("Unexpected hello message.\n");
    return -1;
  }

  for (curr = ctx->clients->head; curr != NULL; curr = curr->next) {
    struct client_tcp* c = curr->data;

    if (strcmp(c->id, id) == 0) {
      if (c->sockfd == -1) {
        c->sockfd = sockfd;
        c->addr = client->addr;
        c->addrlen = client->addrlen;

        printf("Client %s reconnected.\n", id);

        list_delete(ctx->pending, curr);
      } else {
        // eroare
        printf("Username is already in use.\n");
        return -1;
      }
    } else {
      memcpy(client->id, id, sizeof(client->id));
      list_insert(ctx->clients, client, ctx->clients->tail);

      printf("Client %s connected.\n", id);
    }
  }

  return 0;
}

int handle_tcp_message(struct context* ctx, int sockfd)
{
  char buf[MAX_PAYLOAD_LEN] = {0};
  int n_read = read(sockfd, buf, sizeof(buf));
  if (n_read == -1) {
    fprintf(stderr, "Read failed.");
    return -1;
  }

  if (n_read == 0) {
    // TODO disconnect client.
    printf("Client disconnected.\n");
    return 0;
  }

  for (int i = 0; i < n_read; i++)
  {
    printf("%02x ", buf[i] & 0xff);
  }
  printf("\n");

  struct tcp_message_header* hdr = (struct tcp_message_header*) buf;
  char* payload = buf + sizeof(struct tcp_message_header);

  printf("header type = %d\n", hdr->type);
  switch (hdr->type) {
    case HELLO:
    {
      printf("Message type hello\n");
      handle_hello_message(ctx, sockfd, payload);
      break;
    }
  }

  return n_read;
}
