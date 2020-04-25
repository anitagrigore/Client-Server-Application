#pragma once

#include <string>
#include <cstdint>
#include <utility> 
#include <vector>
#include <queue>

#include "data_limits.h"
#include "udp_utils.h"

using Subscription = std::pair<std::string, bool>;

struct TCPClient
{
  std::string id;
  struct sockaddr_storage addr;
  socklen_t addrlen;
  int sockfd;
  std::vector<Subscription> subscriptions;
  std::queue<UDPMessage> pending_messages;
  
  bool is_subscribed_to(std::string topic) const
  {
    for (auto sub = subscriptions.begin(); sub != subscriptions.end(); sub++) {
      if (sub->first == topic) {
        return true;
      }
    }
    
    return false;
  }
  
  std::string addr_str() const
  {
    char host[NI_MAXHOST] = {0};
    char service[NI_MAXSERV] = {0};
    int err = getnameinfo((struct sockaddr*) &addr, addrlen, host, sizeof(host), service, sizeof(service),
        NI_NUMERICHOST | NI_NUMERICSERV);
  
    if (err != 0) {
      return "";
    }
    
    char buf[NI_MAXHOST + NI_MAXSERV + 2]{};
    snprintf(buf, sizeof(buf), "%s:%s", host, service);
    
    return buf;
  }
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
