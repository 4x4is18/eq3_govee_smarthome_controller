// Message.h
#pragma once

#define TOPIC_LEN   64
#define PAYLOAD_LEN 32

struct Message {
  char topic[TOPIC_LEN];
  char payload[PAYLOAD_LEN];
};