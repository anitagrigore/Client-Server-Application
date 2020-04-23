#pragma once

#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PAYLOAD_LEN 1500
#define MAX_ID_LEN 10
#define MAX_TOPIC_LEN 50

#define DIE(assertion, call_description)	\
	do {									                  \
		if (assertion) {					            \
			fprintf(stderr, "(%s, %d): ",	      \
					__FILE__, __LINE__);	          \
			perror(call_description);		        \
			exit(EXIT_FAILURE);				          \
		}									                    \
	} while(0)

struct int_msg
{
  uint8_t sign;
  uint32_t value;
} __attribute__((packed));

struct short_real_msg
{
  uint16_t value;
} __attribute__((packed));

struct float_msg
{
  uint8_t sign;
  uint32_t value;
  uint8_t modulo;
} __attribute__((packed));

struct udp_message_header
{
  // The topic in which the message will be posted
  char topic[MAX_TOPIC_LEN];

  // The numerical value of the type identifier
  uint8_t type;

  union {
    char string_msg[MAX_PAYLOAD_LEN];
    struct int_msg int_msg;
    struct short_real_msg short_real_msg;
    struct float_msg float_msg;
  };
} __attribute__((packed));

struct address_header
{
  // Source address and the length of it
  struct sockaddr_storage addr;
  socklen_t addrlen;
};

struct udp_message
{
  struct udp_message_header message;
  struct address_header addr;
};

struct context
{
  struct list* message_list;
  struct list* clients;
  struct list* pending;
};

typedef enum
{
  INT,
  SHORT_REAL,
  FLOAT,
  STRING,
} udp_message_type;

struct client_tcp
{
  char id[MAX_ID_LEN + 1];
  struct sockaddr_storage addr;
  socklen_t addrlen;
  int sockfd;
};

typedef enum
{
  HELLO,
  SUBSCRIBE,
  UNSUBSCRIBE,
} tcp_message_type;

struct tcp_message_header
{
  uint8_t type;
  uint16_t size;
} __attribute__((packed));
