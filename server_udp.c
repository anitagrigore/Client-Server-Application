#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "helper.h"
#include "linkedlist.h"
#include "server_udp.h"

int server_udp(uint16_t port)
{
  int sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd_udp == -1) {
    //eroare
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd_udp, (struct sockaddr*) &server, sizeof(server)) == -1) {
    //error
  }

  return sockfd_udp;
}

int handle_udp_message(struct context* ctx, int sockfd_udp)
{
  struct udp_message p = {0};
  p.addr.addrlen = sizeof(p.addr.addr);

  ssize_t n_recv = recvfrom(sockfd_udp, &p.message, sizeof(p.message), 0,
        (struct sockaddr*) &p.addr.addr, &p.addr.addrlen);
  if (n_recv == -1) {
    //eroare
    return -1;
  }

  list_insert(ctx->message_list, &p, ctx->message_list->tail);

  print_udp_message(&p);

  return 0;
}

int print_udp_message(struct udp_message *p)
{
  char host[NI_MAXHOST] = {0};
  char service[NI_MAXSERV] = {0};
  int err = getnameinfo((struct sockaddr*) &p->addr.addr, p->addr.addrlen, host, sizeof(host), service, sizeof(service),
      NI_NUMERICHOST | NI_NUMERICSERV);

  if (err != 0) {
    fprintf(stderr, "Error: %s\n", gai_strerror(err));
  }

  char topic[MAX_TOPIC_LEN + 1] = {0};
  memcpy(topic, p->message.topic, sizeof(p->message.topic));

  printf("%s:%s - %s ", host, service, topic);

  switch (p->message.type) {
    case INT:
    {
      uint32_t value = ntohl(p->message.int_msg.value);

      printf("- INT - %s%d", p->message.int_msg.sign ? "-" : "", value);
      break;
    }
    case SHORT_REAL:
    {
      uint16_t value = ntohs(p->message.short_real_msg.value);
      uint16_t fractional = value % 100;
      uint16_t integral = value / 100;

      printf("- SHORT_REAL - %d.%02d", integral, fractional);
      break;
    }
    case FLOAT:
    {
      uint32_t value = ntohl(p->message.float_msg.value);
      uint8_t modulo = p->message.float_msg.modulo;
      uint32_t p10 = pow(10, modulo);
      uint32_t fractional = value % p10;
      uint32_t integral = value / p10;

      printf("- FLOAT - %s%d.%0*d", p->message.float_msg.sign ? "-" : "", integral,
          modulo, fractional);
      break;
    }
    case STRING:
    {
      char buf[MAX_PAYLOAD_LEN + 1] = {0};
      memcpy(buf, p->message.string_msg, sizeof(p->message.string_msg));

      printf("- STRING - %s", buf);
      break;
    }
  }

  printf("\n");
  return 0;
}
