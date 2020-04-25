#pragma once

#include <cstdint>
#include <string>

#include "context_manager.h"
#include "helper.h"

class Server_TCP {
  int port;
  ContextManager &ctx;
  int server_sock = -1;

public:
  Server_TCP(ContextManager &ctx, int port) : ctx{ctx}, port{port} {}

  // Open tcp socket and call bind and listen on it.
  int start();

  // Accept client and add it to the list of pending clients.
  int handle_client();

  // Read the message from client and call the corresponding functions.
  int handle_message(int clientfd);

  // Forward the messages from the pending messages queue.
  int share_messages();

  int get_fd() const { return server_sock; }

private:
  // If an existent and active client sends a hello message, erase it form the
  // pending list and return -1. If an existent but inactive client send a hello
  // message, change the addres and file descriptor with the ones from the
  // message and remove it from the pending list. If it's a new client, add it
  // to the client list.
  int handle_hello_message(int clientfd, const std::string &id);

  // If a subscription already exits, return. Otherwise, add it to the list of
  // subscription for this client.
  int handle_subscribe_message(int clientfd, const std::string &topic,
                               bool store_forward);

  // If a subscription does not exist, ignore it. Otherwise, remove it from the
  // list of subscriptions for this client.
  int handle_unsubscribe_message(int clientfd, const std::string &topic);

  // If the client has store and forward set, keep the information. Otherwise,
  // remove all the data.
  void disconnect_client(int clientfd);
};
