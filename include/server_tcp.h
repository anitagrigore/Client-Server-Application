#pragma once
#include <stdint.h>

#include "helper.h"
#include "linkedlist.h"

int server_tcp(uint16_t port_nr);

int handle_tcp_client(struct context* ctx, int sockfd_tcp);

int handle_tcp_message(struct context* ctx, int sockfd);

int handle_hello_message(struct context* ctx, int sockfd, char* id);
