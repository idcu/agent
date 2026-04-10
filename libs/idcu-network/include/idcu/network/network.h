#ifndef IDCU_NETWORK_NETWORK_H
#define IDCU_NETWORK_NETWORK_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/network/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_network_init(idcu_Network_Context** ctx);
void idcu_network_destroy(idcu_Network_Context* ctx);
int idcu_network_is_initialized(idcu_Network_Context* ctx);
uint64_t idcu_network_get_operation_count(idcu_Network_Context* ctx);
uint64_t idcu_network_get_error_count(idcu_Network_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
