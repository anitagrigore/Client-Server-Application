#pragma once

#include <string>
#include <cstdint>

#include "helper.h"
#include "context_manager.h"

class Server_TCP
{
  int port;
  ContextManager &ctx;
  int server_sock = -1;
  
public:
  Server_TCP(ContextManager &ctx, int port) : ctx{ctx}, port{port}
  {}
  
  int start();
  
  int handle_client();
  int handle_message(int clientfd);
  
  int get_fd() const
  {
    return server_sock;
  }
  
private:
  int handle_hello_message(int clientfd, const std::string &id);
  int handle_subscribe_message(int clientfd, const std::string &topic, bool store_forward);
  int handle_unsubscribe_message(int clientfd, const std::string &topic);
  
  void disconnect_client(int clientfd);
};
