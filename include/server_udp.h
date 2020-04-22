#pragma once
#include "helper.h"
#include <stdint.h>

int server_udp(uint16_t port);

int handle_udp_message(struct context* ctx, int sockfd_udp);

int print_udp_message(struct udp_message *p);
