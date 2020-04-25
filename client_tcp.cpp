#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <cassert>

#include "client_tcp.h"
#include "string_utils.h"
#include "tcp_utils.h"
#include "data_limits.h"
#include "helper.h"
#include "udp_utils.h"

int Client_TCP::start()
{
  int ret;
  
  int sockfd_cli = socket(AF_INET, SOCK_STREAM, 0);
  DIE(sockfd_cli < 0, "socket");
  
  sockaddr_in cli_addr;
  cli_addr.sin_family = AF_INET;
  cli_addr.sin_port = htons(port);
  ret = inet_aton(ip.c_str(), &cli_addr.sin_addr);
  DIE(ret < 0, "inet_aton");
  
  ret = connect(sockfd_cli, (sockaddr*) &cli_addr, sizeof(cli_addr));
  DIE(ret < 0, "connect");
 
  fd = sockfd_cli;
  
  return sockfd_cli;
}

int Client_TCP::say_hello(std::string id)
{
  char buf[sizeof(TCPMessageHeader) + MAX_ID_LEN]{};
  
  auto payload_len = std::min<size_t>(MAX_ID_LEN, id.size());
  
  TCPMessageHeader *hdr = (TCPMessageHeader *) buf;
  hdr->type = HELLO;
  hdr->size = htons(payload_len);
  
  char *payload = buf + sizeof(TCPMessageHeader);
  memcpy(payload, id.c_str(), payload_len);
  
  auto buf_size = sizeof(TCPMessageHeader) + payload_len;
  send_to_server(buf, buf_size);
  
  return 0;
}

int Client_TCP::send_to_server(void *buf, size_t n)
{
  int n_send = send(fd, buf, n, 0);
  DIE(n_send < 0, "send");
  
  return 0;
}

int Client_TCP::handle_command()
{
  char buf[128]{0};
  ssize_t n_read = read(STDIN_FILENO, buf, sizeof(buf));
  if (n_read == -1) {
    perror("read");
    return -1;
  }
  
  char *newline = strchr(buf, '\n');
  assert(newline != nullptr);
  
  size_t line_length = newline - buf;
  std::string line(buf, line_length);
  
  auto tokens = split(line, ' ');
  auto &command = tokens.front();
  
  if (command == "exit") {
    return 0;
  }
  
  if (command == "subscribe") {
    if (tokens.size() != 3) {
      std::cerr << "usage: subscribe topic sf\n";
      return -1;
    }
    
    bool sf = std::atoi(tokens[2].c_str());
    
    char cmd_buf[sizeof(TCPMessageHeader) + sizeof(TCPSubscribeMsg)]{};
    auto hdr = (TCPMessageHeader *) cmd_buf;
    auto msg = (TCPSubscribeMsg *) (cmd_buf + sizeof(TCPMessageHeader));
    
    hdr->type = SUBSCRIBE;
    hdr->size = htons(sizeof(TCPSubscribeMsg));
    
    memcpy(msg->topic, tokens[1].c_str(), std::min<size_t>(MAX_TOPIC_LEN, tokens[1].size()));
    msg->sf = sf;
    
    if (send_to_server(cmd_buf, sizeof(cmd_buf)) < 0) {
      std::cerr << "Failed to send message.\n";
      return -1;
    } else {
      std::cout << "Subscribed to " << tokens[1] << "\n";
    }
    
    return 1;
  }
  
  if (command == "unsubscribe") {
    if (tokens.size() != 2) {
      std::cerr << "usage: unsubscribe topic\n";
      return -1;
    }
    
    char cmd_buf[sizeof(TCPMessageHeader) + sizeof(TCPUnsubscribeMsg)]{};
    auto hdr = (TCPMessageHeader *) cmd_buf;
    auto msg = (TCPUnsubscribeMsg *) (cmd_buf + sizeof(TCPMessageHeader));
    
    hdr->type = UNSUBSCRIBE;
    hdr->size = htons(sizeof(TCPUnsubscribeMsg));

    memcpy(msg->topic, tokens[1].c_str(), std::min<size_t>(MAX_TOPIC_LEN, tokens[1].size()));

    if (send_to_server(cmd_buf, sizeof(cmd_buf)) < 0) {
      std::cerr << "Failed to send message.\n";
      return -1;
    } else {
      std::cout << "Subscribed to " << tokens[1] << "\n";
    }
    
    return 1;
  }
  
  std::cerr << "Unknown command\n";
  return -1;
}

int Client_TCP::handle_server_message()
{
  char buf[2048]{};
  
  ssize_t nread = read(fd, buf, sizeof(buf));
  if (nread == -1)
  {
    perror("read");
    return -1;
  }
  
  if (nread == 0)
  {
    return 0;
  }

  // Concatenate the buffer being read with the data fragment.
  uint8_t *temp = (uint8_t *) calloc(fragment_size + nread, 1);
  if (fragment_size) {
    memcpy(temp, data_fragment, fragment_size);
  }
  memcpy(temp + fragment_size, buf, nread);
  fragment_size += nread;
  
  free(data_fragment);
  data_fragment = temp;

  while (can_parse_message(data_fragment, fragment_size)) {
    auto preamble = (UDPPreamble *) data_fragment;
    auto payload = (char *) (data_fragment + sizeof(UDPPreamble));

    std::cout << payload << "\n";

    // Trim what was parsed from the data fragment
    auto parsed_size = ntohs(preamble->msg_len) + sizeof(UDPPreamble);
    if (fragment_size - parsed_size == 0) {
      free(data_fragment);
      data_fragment = nullptr;
      fragment_size = 0;
      
      break;
    }

    temp = (uint8_t *) malloc(fragment_size - parsed_size);
    if (!temp) {
      perror("malloc");
      return -1;
    }
    
    memcpy(temp, data_fragment + parsed_size, fragment_size - parsed_size);
    free(data_fragment);
    data_fragment = temp;
    fragment_size -= parsed_size;
  }
  
  return 1;
}
