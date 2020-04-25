#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <netinet/tcp.h>

#include "server_tcp.h"
#include "tcp_utils.h"

#define BACKLOG 1024

int Server_TCP::start()
{
  int sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd_tcp < 0) {
    perror("socket_tcp");
    return -1;
  }
  
  int val = 1;
  if (setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
  {
    close(sockfd_tcp);
    perror("setsockopt");
    return -1;
  }
  
  int f = 1;
  if (setsockopt(sockfd_tcp, IPPROTO_TCP, TCP_NODELAY, &f, sizeof(f)) == -1)
  {
    close(sockfd_tcp);
    perror("setsockopt");
    return -1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd_tcp, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("bind");
    return -1;
  }

  if (listen(sockfd_tcp, BACKLOG) < 0) {
    perror("listen");
    return -1;
  }
  
  server_sock = sockfd_tcp;

  return sockfd_tcp;
}

int Server_TCP::handle_client()
{
  TCPClient pending_conn{};
  pending_conn.addrlen = sizeof(struct sockaddr_storage);

  int clientfd = accept(server_sock, (struct sockaddr*) &pending_conn.addr, &pending_conn.addrlen);
  if (clientfd < 0) {
    perror("accept");
    return -1;
  }

  pending_conn.sockfd = clientfd;
  ctx.pending_conns.push_back(pending_conn);

  return clientfd;
}

int Server_TCP::handle_message(int clientfd)
{
  char buf[MAX_PAYLOAD_LEN] = {0};
  int n_read = read(clientfd, buf, sizeof(buf));
  if (n_read == -1) {
    perror("handle_message: read");
    return -1;
  }

  if (n_read == 0) {
    disconnect_client(clientfd);
    return 0;
  }

  TCPMessageHeader* hdr = (TCPMessageHeader*) buf;
  char* payload = buf + sizeof(TCPMessageHeader);

  switch (hdr->type) {
    case HELLO:
    {
      char id[MAX_ID_LEN + 1]{};
      memcpy(id, payload, sizeof(std::min<size_t>(MAX_ID_LEN, ntohs(hdr->size))));
      
      handle_hello_message(clientfd, std::string(id));
      break;
    }
    case SUBSCRIBE:
    {
      auto subscribe_msg = (TCPSubscribeMsg *) payload;
      char topic[MAX_TOPIC_LEN + 1]{};
      memcpy(topic, subscribe_msg->topic, MAX_TOPIC_LEN);
      
      handle_subscribe_message(clientfd, std::string(topic), subscribe_msg->sf);
      break;
    }
    case UNSUBSCRIBE:
    {
      auto unsubscribe_msg = (TCPUnsubscribeMsg *) payload;
      char topic[MAX_TOPIC_LEN + 1]{};
      memcpy(topic, unsubscribe_msg->topic, MAX_TOPIC_LEN);
      
      handle_unsubscribe_message(clientfd, std::string(topic));
      break;
    }
  }

  return n_read;
}

int Server_TCP::share_messages()
{
  while (!ctx.pending_messages.empty()) {
    auto &msg = ctx.pending_messages.front();
    char buf[3072]{};
    auto n = msg.serialize(buf);
    
    printf("Sharing message on topic: %s\n", msg.message.topic);

    for (auto &client : ctx.clients) {
      if (!client.is_subscribed_to(msg.message.topic)) {
        continue;
      }
      
      printf("Client %s is subscribed to topic.\n", client.id.c_str());

      if (client.sockfd == -1) {
        printf("Client is offline. Keeping the message there.\n");
        
        client.pending_messages.push(msg);
      } else {
        printf("Sending message.\n");

        if (write(client.sockfd, buf, n) == -1) {
          perror("send");

          return -1;
        }
      }
    }
    
    ctx.pending_messages.pop();
  }
  
  return 0;
}

int Server_TCP::handle_hello_message(int clientfd, const std::string &id)
{
  auto pending_conn = ctx.pending_conns.end();

  for (auto it = ctx.pending_conns.begin(); it != ctx.pending_conns.end(); it++) {
    if (it->sockfd == clientfd) {
      pending_conn = it;
      break;
    }
  }

  if (pending_conn == ctx.pending_conns.end()) {
    std::cerr << "Unexpected hello message.\n";
    return -1;
  }

  for (auto &c : ctx.clients) {
    if (c.id == id) {
      if (c.sockfd == -1) {
        c.sockfd = pending_conn->sockfd;
        c.addr = pending_conn->addr;
        c.addrlen = pending_conn->addrlen;

        std::cout << "Client " << id << " reconnected.\n";

        ctx.pending_conns.erase(pending_conn);
        
        while (!c.pending_messages.empty())
        {
          auto &msg = c.pending_messages.front();
          char buf[2048]{};
          auto n = msg.serialize(buf);
          
          if (write(c.sockfd, buf, n) == -1) {
            perror("send");
  
            return -1;
          }
          
          c.pending_messages.pop();
        }
        return 0;
      } else {
        std::cerr << "Username is already in use.\n";
        ctx.pending_conns.erase(pending_conn);
        return -1;
      }
    }
  }
  
  TCPClient client = *pending_conn;
  client.id = id;

  ctx.pending_conns.erase(pending_conn); // Remove the pending connection
  ctx.clients.push_back(client);  // Add the new client to known clients list.

  std::cout << "Client " << id << " connected.\n";

  return 0;
}

int Server_TCP::handle_subscribe_message(int clientfd, const std::string &topic, bool sf)
{
  auto is_pending = std::find_if(
      ctx.pending_conns.begin(),
      ctx.pending_conns.end(),
      [&](auto &&c) { return c.sockfd == clientfd; }) != ctx.pending_conns.end();

  if (is_pending) {
    // Expecting HELLO message from pending connection instead.
    return -1;
  }
  
  auto client = std::find_if(
      ctx.clients.begin(),
      ctx.clients.end(),
      [&](auto &&c) { return c.sockfd == clientfd; });
  
  assert(client != ctx.clients.end());
  
  auto has_subscription = std::find_if(
      client->subscriptions.begin(),
      client->subscriptions.end(),
      [&](auto &&c) { return c.first == topic; }) != client->subscriptions.end();
  if (has_subscription)
  {
    // A subscription already exists. Silently ignore. Treat as success.
    printf("Client %s is already subscribed to topic %s\n", client->id.c_str(), topic.c_str());
    return 0;
  }
  
  printf("Client %s subscribed to topic %s\n.", client->id.c_str(), topic.c_str());
  client->subscriptions.emplace_back(topic, sf);
  return 0;
}

int Server_TCP::handle_unsubscribe_message(int clientfd, const std::string &topic)
{
  auto is_pending = std::find_if(
      ctx.pending_conns.begin(),
      ctx.pending_conns.end(),
      [&](auto &&c) { return c.sockfd == clientfd; }) != ctx.pending_conns.end();

  if (is_pending) {
    // Expecting HELLO message from pending connection instead.
    return -1;
  }
  
  auto client = std::find_if(
      ctx.clients.begin(),
      ctx.clients.end(),
      [&](auto &&c) { return c.sockfd == clientfd; });
  
  assert(client != ctx.clients.end());
  
  auto subscription = std::find_if(
      client->subscriptions.begin(),
      client->subscriptions.end(),
      [&](auto &&c) { return c.first == topic; });
  if (subscription == client->subscriptions.end())
  {
    // A subscription does not exist. Silently ignore this.
    return 0;
  }

  client->subscriptions.erase(subscription);
  return 0;
}

void Server_TCP::disconnect_client(int clientfd)
{
  auto pending_conn = std::find_if(
      ctx.pending_conns.begin(),
      ctx.pending_conns.end(),
      [&](auto &&c) { return c.sockfd == clientfd; });
  if (pending_conn != ctx.pending_conns.end()) {
    std::cout << "Disconnected pending connection: " << clientfd << "\n";
    
    ctx.pending_conns.erase(pending_conn);
    return;
  }

  // The client has been registered already. Clean their subscriptions.
  
  auto client = std::find_if(
      ctx.clients.begin(),
      ctx.clients.end(),
      [&](auto &&c) { return c.sockfd == clientfd; });
  
  assert(client != ctx.clients.end());
  
  bool has_sf_subscrptions = false;
  for (auto it = client->subscriptions.begin(); it != client->subscriptions.end();) {
    auto &topic = it->first;
    auto sf = it->second;
    
    if (!sf) {
      std::cout << "Removed subscription (" << client->id << ", " << topic << ")\n";
      it = client->subscriptions.erase(it);
    } else {
      has_sf_subscrptions = true;
      it++;
    }
  }
  
  if (!has_sf_subscrptions) {
    std::cout << "Removing client information for " << client->id << "\n";
  
    ctx.clients.erase(client);
    return;
  }
  
  std::cout << "Keeping client info due to SF subscriptions: " << client->id << "\n";
  
  client->sockfd = -1;
}
