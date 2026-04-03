#ifndef IDCU_SECURITY_SANDBOX_H
#define IDCU_SECURITY_SANDBOX_H

#include "scheduler/context.h"
#include "common/config.h"

#define IDCU_PERM_SEND    (1U << 0)
#define IDCU_PERM_RECV    (1U << 1)
#define IDCU_PERM_RUN     (1U << 2)
#define IDCU_PERM_HW      (1U << 3)

typedef struct {
    uint32_t module_id;
    uint32_t perm;
    uint32_t quota;
    idcu_StackContext ctx;
} idcu_Sandbox;

static inline int idcu_sandbox_perm_check(idcu_Sandbox *sb, uint32_t mask)
{
    return ((sb->perm & mask) == mask) ? 0 : -1;
}

#endif // IDCU_SECURITY_SANDBOX_H
