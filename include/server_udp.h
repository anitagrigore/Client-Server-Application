#pragma once

#include "context_manager.h"
#include "helper.h"

#include <cstdint>

class Server_UDP {
  int port;
  ContextManager &ctx;
  int server_sock = -1;

public:
  Server_UDP(ContextManager &ctx, int port) : ctx{ctx}, port{port} {}

  // Open socket and call bind. Returns the file descriptor.
  int start();

  // Add the message received from an udp client to the pending messages queue.
  int handle_message();

  int get_fd() const { return server_sock; }
};
