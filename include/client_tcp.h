#pragma once 

#include <string>

class Client_TCP
{
  int port;
  int fd = -1;
  std::string ip;
  
public:
  Client_TCP(std::string ip, int port) : ip{ip}, port{port}
  {}
  
  int start();
  int say_hello(std::string id);
  int handle_server_message();
  int handle_command();
  int send_to_server(void *buf, size_t n);
  
  int get_fd() 
  {
    return fd;
  }
};