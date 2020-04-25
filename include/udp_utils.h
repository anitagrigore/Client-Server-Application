#pragma once

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "data_limits.h"

enum UDPMessageType
{
  INT,
  SHORT_REAL,
  FLOAT,
  STRING,
};

struct IntMessage
{
  uint8_t sign;
  uint32_t value;
} __attribute__((packed));

struct ShortRealMessage
{
  uint16_t value;
} __attribute__((packed));

struct FloatMessage
{
  uint8_t sign;
  uint32_t value;
  uint8_t modulo;
} __attribute__((packed));

struct UDPMessageHeader
{
  // The topic in which the message will be posted
  char topic[MAX_TOPIC_LEN];

  // The numerical value of the type identifier
  uint8_t type;

  union {
    char string_msg[MAX_PAYLOAD_LEN];
    IntMessage int_msg;
    ShortRealMessage short_real_msg;
    FloatMessage float_msg;
  };
} __attribute__((packed));

struct AddressHeader
{
  // Source address and the length of it
  struct sockaddr_storage addr;
  socklen_t addrlen;
};

struct UDPMessage
{
  UDPMessageHeader message;
  AddressHeader source;

  void print()
  {
    char host[NI_MAXHOST] = {0};
    char service[NI_MAXSERV] = {0};
    int err = getnameinfo((struct sockaddr*) &source.addr, source.addrlen, host, sizeof(host), service, sizeof(service),
        NI_NUMERICHOST | NI_NUMERICSERV);
  
    if (err != 0) {
      fprintf(stderr, "Error: %s\n", gai_strerror(err));
      return;
    }
  
    char topic[MAX_TOPIC_LEN + 1] = {0};
    memcpy(topic, message.topic, sizeof(message.topic));
  
    printf("%s:%s - %s ", host, service, topic);
  
    switch (message.type) {
      case INT:
      {
        uint32_t value = ntohl(message.int_msg.value);
  
        printf("- INT - %s%d", message.int_msg.sign ? "-" : "", value);
        break;
      }
      case SHORT_REAL:
      {
        uint16_t value = ntohs(message.short_real_msg.value);
        uint16_t fractional = value % 100;
        uint16_t integral = value / 100;
  
        printf("- SHORT_REAL - %d.%02d", integral, fractional);
        break;
      }
      case FLOAT:
      {
        uint32_t value = ntohl(message.float_msg.value);
        uint8_t modulo = message.float_msg.modulo;
        uint32_t p10 = pow(10, modulo);
        uint32_t fractional = value % p10;
        uint32_t integral = value / p10;
  
        printf("- FLOAT - %s%d.%0*d", message.float_msg.sign ? "-" : "", integral,
            modulo, fractional);
        break;
      }
      case STRING:
      {
        char buf[MAX_PAYLOAD_LEN + 1] = {0};
        memcpy(buf, message.string_msg, sizeof(message.string_msg));
  
        printf("- STRING - %s", buf);
        break;
      }
    }
  
    printf("\n");
  }
};
