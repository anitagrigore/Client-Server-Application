#pragma once

#include <string>
#include <cstdint>

#include "data_limits.h"

struct TCPClient
{
  std::string id;
  struct sockaddr_storage addr;
  socklen_t addrlen;
  int sockfd;
};

enum TCPMessageType
{
  HELLO = 0,
  SUBSCRIBE = 1,
  UNSUBSCRIBE = 2,
};

struct TCPMessageHeader
{
  uint8_t type;
  uint16_t size;
} __attribute__((packed));
