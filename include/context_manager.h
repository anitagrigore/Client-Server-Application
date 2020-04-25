#pragma once

#include "helper.h"
#include "tcp_utils.h"
#include "udp_utils.h"

#include <list>
#include <queue>

struct ContextManager {
  // List of all clients.
  std::list<TCPClient> clients;

  // List of pending connections.
  std::list<TCPClient> pending_conns;
  std::queue<UDPMessage> pending_messages;
};
