#ifndef IDCU_SANDBOX_SANDBOX_H
#define IDCU_SANDBOX_SANDBOX_H

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_PERM_SEND    (1U << 0)
#define IDCU_PERM_RECV    (1U << 1)
#define IDCU_PERM_RUN     (1U << 2)
#define IDCU_PERM_HW      (1U << 3)
#define IDCU_PERM_FILE    (1U << 4)
#define IDCU_PERM_NETWORK (1U << 5)
#define IDCU_PERM_ALL                                                                  \
    (IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN | IDCU_PERM_HW | IDCU_PERM_FILE | \
     IDCU_PERM_NETWORK)

#define IDCU_SANDBOX_DEFAULT_MEMORY_LIMIT  (1024 * 1024 * 128)
#define IDCU_SANDBOX_DEFAULT_CPU_LIMIT     50
#define IDCU_SANDBOX_DEFAULT_NETWORK_LIMIT (1024 * 1024 * 10)

typedef struct
{
    uint64_t memory_used;
    uint64_t memory_limit;
    uint32_t cpu_usage_percent;
    uint32_t cpu_limit_percent;
    uint64_t network_rx_bytes;
    uint64_t network_tx_bytes;
    uint64_t network_limit_bytes;
} idcu_ResourceUsage;

typedef struct
{
    uint32_t           module_id;
    uint32_t           perm;
    uint32_t           quota;
    idcu_ResourceUsage resources;
} idcu_Sandbox;

int      idcu_sandbox_init(idcu_Sandbox* sb, uint32_t module_id, uint32_t initial_perm);
void     idcu_sandbox_destroy(idcu_Sandbox* sb);
int      idcu_sandbox_set_perm(idcu_Sandbox* sb, uint32_t perm);
int      idcu_sandbox_add_perm(idcu_Sandbox* sb, uint32_t perm);
int      idcu_sandbox_remove_perm(idcu_Sandbox* sb, uint32_t perm);
int      idcu_sandbox_perm_check(idcu_Sandbox* sb, uint32_t mask);
uint32_t idcu_sandbox_get_perm(idcu_Sandbox* sb);
int      idcu_sandbox_set_quota(idcu_Sandbox* sb, uint32_t quota);
uint32_t idcu_sandbox_get_quota(idcu_Sandbox* sb);

int idcu_sandbox_set_memory_limit(idcu_Sandbox* sb, uint64_t limit_bytes);
int idcu_sandbox_set_cpu_limit(idcu_Sandbox* sb, uint32_t limit_percent);
int idcu_sandbox_set_network_limit(idcu_Sandbox* sb, uint64_t limit_bytes);

int idcu_sandbox_check_memory(idcu_Sandbox* sb, uint64_t requested_bytes);
int idcu_sandbox_check_cpu(idcu_Sandbox* sb);
int idcu_sandbox_check_network(idcu_Sandbox* sb, uint64_t requested_bytes);

int idcu_sandbox_update_memory_usage(idcu_Sandbox* sb, int64_t delta_bytes);
int idcu_sandbox_update_network_rx(idcu_Sandbox* sb, uint64_t bytes);
int idcu_sandbox_update_network_tx(idcu_Sandbox* sb, uint64_t bytes);

int idcu_sandbox_get_resource_usage(idcu_Sandbox* sb, idcu_ResourceUsage* usage);

#define IDCU_MAX_SANDBOXES 64

int           idcu_sandbox_registry_init(void);
void          idcu_sandbox_registry_destroy(void);
int           idcu_sandbox_registry_add(idcu_Sandbox* sb);
int           idcu_sandbox_registry_remove(uint32_t module_id);
idcu_Sandbox* idcu_sandbox_registry_get(uint32_t module_id);
int           idcu_sandbox_registry_check_perm(uint32_t module_id, uint32_t mask);
int           idcu_sandbox_registry_check_memory(uint32_t module_id, uint64_t requested_bytes);
int           idcu_sandbox_registry_check_network(uint32_t module_id, uint64_t requested_bytes);

#endif  // IDCU_SANDBOX_SANDBOX_H
