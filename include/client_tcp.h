#pragma once

#include <string>

class Client_TCP {
  int port;
  int fd = -1;
  std::string ip;

  uint8_t *data_fragment = nullptr;
  size_t fragment_size = 0;

public:
  Client_TCP(std::string ip, int port)
      : ip{ip}, port{port}, data_fragment{nullptr}, fragment_size{0} {}

  // Open socket and bind connect to server.
  int start();

  // Send a hello message containing the id of the client.
  int say_hello(std::string id);

  // Forward the message from the udp clients.
  int handle_server_message();

  // Handle subscribe and unsubscribe messages and act accordingly.
  int handle_command();

  // Send the message stored in \param buf to the tcp server.
  int send_to_server(void *buf, size_t n);

  int get_fd() { return fd; }
};
