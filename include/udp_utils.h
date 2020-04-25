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

  std::string str() const
  {
    char buf[2048]{};
    std::string output;
    
    char host[NI_MAXHOST] = {0};
    char service[NI_MAXSERV] = {0};
    int err = getnameinfo((struct sockaddr*) &source.addr, source.addrlen, host, sizeof(host), service, sizeof(service),
        NI_NUMERICHOST | NI_NUMERICSERV);
  
    if (err != 0) {
      return output;
    }
  
    char topic[MAX_TOPIC_LEN + 1] = {0};
    memcpy(topic, message.topic, sizeof(message.topic));
  
    sprintf(buf, "%s:%s - %s ", host, service, topic);
    output.append(buf, strlen(buf));
  
    switch (message.type) {
      case INT:
      {
        uint32_t value = ntohl(message.int_msg.value);
  
        sprintf(buf, "- INT - %s%d", message.int_msg.sign ? "-" : "", value);
        output.append(buf, strlen(buf));
        break;
      }
      case SHORT_REAL:
      {
        uint16_t value = ntohs(message.short_real_msg.value);
        uint16_t fractional = value % 100;
        uint16_t integral = value / 100;
  
        sprintf(buf, "- SHORT_REAL - %d.%02d", integral, fractional);
        output.append(buf, strlen(buf));
        break;
      }
      case FLOAT:
      {
        uint32_t value = ntohl(message.float_msg.value);
        uint8_t modulo = message.float_msg.modulo;
        uint32_t p10 = pow(10, modulo);
        uint32_t fractional = value % p10;
        uint32_t integral = value / p10;
  
        sprintf(buf, "- FLOAT - %s%d.%0*d", message.float_msg.sign ? "-" : "", integral,
            modulo, fractional);
        output.append(buf, strlen(buf));
        break;
      }
      case STRING:
      {
        char payload[MAX_PAYLOAD_LEN + 1] = {0};
        memcpy(payload, message.string_msg, sizeof(message.string_msg));
  
        sprintf(buf, "- STRING - %s", payload);
        output.append(buf, strlen(buf));
        break;
      }
    }
  
    return output;
  }
  
  size_t serialize(void *out) const
  {
    uint8_t *buf = (uint8_t *) out;
    auto msg_str = str();
    
    auto preamble = (UDPPreamble *) buf;
    auto msg_body = (char *) (buf + sizeof(UDPPreamble));

    preamble->msg_len = htons(msg_str.size());
    memcpy(msg_body, msg_str.c_str(), msg_str.size());

    return msg_str.size() + sizeof(UDPPreamble);
  }
};

static bool can_parse_message(uint8_t *buf, size_t len)
{
  if (len <= sizeof(UDPPreamble))
  {
    return false;
  }
  
  auto preamble = (UDPPreamble *) buf;
  return len - sizeof(UDPPreamble) >= ntohs(preamble->msg_len);
}
