#ifndef SANDBOX_H
#define SANDBOX_H

#include "context.h"

#define PERM_SEND    (1U << 0)
#define PERM_RECV    (1U << 1)
#define PERM_RUN     (1U << 2)
#define PERM_HW      (1U << 3)

typedef struct {
    uint32_t module_id;
    uint32_t perm;
    uint32_t quota;
    StackContext ctx;
} Sandbox;

static inline int sandbox_perm_check(Sandbox *sb, uint32_t mask)
{
    return ((sb->perm & mask) == mask) ? 0 : -1;
}

#endif