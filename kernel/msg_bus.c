#include "msg_bus.h"

void msg_bus_init(MessageBus *bus)
{
    memset(bus, 0, sizeof(MessageBus));
}

int msg_send(MessageBus *bus, uint32_t mod_id, const StackContext *ctx)
{
    uint32_t next = (bus->tail + 1) & MSG_BUS_MASK;
    if (next == bus->head) return -1;

    bus->queue[bus->tail] = *ctx;
    bus->tail = next;
    bus->subs[mod_id >> 5] |= (1U << (mod_id & 0x1F));
    return 0;
}

int msg_recv(MessageBus *bus, uint32_t mod_id, StackContext *ctx)
{
    if (bus->head == bus->tail) return -1;

    *ctx = bus->queue[bus->head];
    bus->head = (bus->head + 1) & MSG_BUS_MASK;
    return 0;
}