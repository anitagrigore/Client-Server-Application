#pragma once

#include "helper.h"
#include "context_manager.h"

#include <cstdint>

class Server_UDP
{
  int port;
  ContextManager &ctx;
  int server_sock = -1;
  
public:
  Server_UDP(ContextManager &ctx, int port) : ctx{ctx}, port{port}
  {}
  
  int start();
  
  int handle_message();
  
  int get_fd() const
  {
    return server_sock;
  }
};
