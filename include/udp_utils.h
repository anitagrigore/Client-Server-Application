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
  } __attribute__((packed));
} __attribute__((packed));

struct UDPPreamble
{
  uint16_t msg_len;
} __attribute__((packed));

struct AddressHeader
{
  // Source address and the length of it
  struct sockaddr_storage addr;
  socklen_t addrlen;
};

struct UDPMessage
{
  AddressHeader source;
  UDPMessageHeader message;

  void print() const
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
  
  size_t serialize(void *out) const
  {
    uint8_t *buf = (uint8_t *) out;
    
    auto preamble = (UDPPreamble *) buf;
    auto msg_source = (AddressHeader *) (buf + sizeof(UDPPreamble));
    auto msg_body = (UDPMessageHeader *) (buf + sizeof(UDPPreamble) + sizeof(AddressHeader));

    preamble->msg_len = 0;
    
    memcpy(msg_source, &source, sizeof(AddressHeader));
    preamble->msg_len += sizeof(AddressHeader);
    
    size_t msg_body_len = 0;
    
    switch (message.type) {
    case INT:
      msg_body_len += sizeof(IntMessage);
      break;
    case SHORT_REAL:
      msg_body_len += sizeof(ShortRealMessage);
      break;
    case FLOAT:
      msg_body_len += sizeof(FloatMessage);
      break;
    case STRING:
      msg_body_len += std::min<size_t>(MAX_PAYLOAD_LEN, strlen(message.string_msg));
      break;
    }
    
    msg_body_len += MAX_TOPIC_LEN + 1;
    preamble->msg_len += msg_body_len;
    
    memcpy(msg_body, &message, msg_body_len);
    
    return preamble->msg_len + sizeof(UDPPreamble);
  }
};
