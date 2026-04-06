#ifndef IDCU_SECURITY_SANDBOX_H
#define IDCU_SECURITY_SANDBOX_H

#include "context.h"
#include "idcu/common/config.h"
#include "idcu/common/error_code.h"
#include <stdint.h>

#define IDCU_PERM_SEND    (1U << 0)
#define IDCU_PERM_RECV    (1U << 1)
#define IDCU_PERM_RUN     (1U << 2)
#define IDCU_PERM_HW      (1U << 3)
#define IDCU_PERM_FILE    (1U << 4)
#define IDCU_PERM_NETWORK (1U << 5)
#define IDCU_PERM_ALL     (IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN | \
                           IDCU_PERM_HW | IDCU_PERM_FILE | IDCU_PERM_NETWORK)

typedef struct {
    uint32_t module_id;
    uint32_t perm;
    uint32_t quota;
    idcu_StackContext ctx;
} idcu_Sandbox;

int idcu_sandbox_init(idcu_Sandbox* sb, uint32_t module_id, uint32_t initial_perm);
void idcu_sandbox_destroy(idcu_Sandbox* sb);
int idcu_sandbox_set_perm(idcu_Sandbox* sb, uint32_t perm);
int idcu_sandbox_add_perm(idcu_Sandbox* sb, uint32_t perm);
int idcu_sandbox_remove_perm(idcu_Sandbox* sb, uint32_t perm);
int idcu_sandbox_perm_check(idcu_Sandbox* sb, uint32_t mask);
uint32_t idcu_sandbox_get_perm(idcu_Sandbox* sb);
int idcu_sandbox_set_quota(idcu_Sandbox* sb, uint32_t quota);
uint32_t idcu_sandbox_get_quota(idcu_Sandbox* sb);

#define IDCU_MAX_SANDBOXES 64

int idcu_sandbox_registry_init(void);
void idcu_sandbox_registry_destroy(void);
int idcu_sandbox_registry_add(idcu_Sandbox* sb);
int idcu_sandbox_registry_remove(uint32_t module_id);
idcu_Sandbox* idcu_sandbox_registry_get(uint32_t module_id);
int idcu_sandbox_registry_check_perm(uint32_t module_id, uint32_t mask);

#endif // IDCU_SECURITY_SANDBOX_H
