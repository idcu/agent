#ifndef MSG_BUS_H
#define MSG_BUS_H

#include "context.h"
#include "include/config.h"

#define MSG_BUS_MASK (CONFIG_MAX_MSG - 1)

typedef struct {
    StackContext queue[CONFIG_MAX_MSG];
    uint32_t head;
    uint32_t tail;
    uint32_t subs[16];
} MessageBus;

void msg_bus_init(MessageBus *bus);
int msg_send(MessageBus *bus, uint32_t mod_id, const StackContext *ctx);
int msg_recv(MessageBus *bus, uint32_t mod_id, StackContext *ctx);

#endif