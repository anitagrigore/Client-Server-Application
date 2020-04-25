#pragma once

#include <string>
#include <cstdint>
#include <utility> 
#include <vector>

#include "data_limits.h"

using Subscription = std::pair<std::string, bool>;

struct TCPClient
{
  std::string id;
  struct sockaddr_storage addr;
  socklen_t addrlen;
  int sockfd;
  std::vector<Subscription> subscriptions;
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

struct TCPSubscribeMsg
{
  char topic[MAX_TOPIC_LEN];
  uint8_t sf;
} __attribute__((packed));

struct TCPUnsubscribeMsg
{
  char topic[MAX_TOPIC_LEN];
};
