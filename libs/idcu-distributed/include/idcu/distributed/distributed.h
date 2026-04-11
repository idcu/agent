#ifndef IDCU_DISTRIBUTED_DISTRIBUTED_H
#define IDCU_DISTRIBUTED_DISTRIBUTED_H

#include "idcu/common/error_code.h"
#include "idcu/distributed/types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_distributed_node_init(idcu_Distributed_Context** ctx);
int idcu_distributed_node_start(idcu_Distributed_Context* ctx);
int idcu_distributed_node_stop(idcu_Distributed_Context* ctx);
void idcu_distributed_node_destroy(idcu_Distributed_Context* ctx);

int idcu_distributed_send_to(idcu_Distributed_Context* ctx, idcu_NodeId target_id, const void* data, size_t data_size);
int idcu_distributed_broadcast(idcu_Distributed_Context* ctx, const void* data, size_t data_size);
int idcu_distributed_get_leader(idcu_Distributed_Context* ctx, idcu_NodeId* leader_id);

int idcu_distributed_get_node_info(idcu_Distributed_Context* ctx, idcu_NodeId node_id, idcu_NodeInfo* info);
int idcu_distributed_list_nodes(idcu_Distributed_Context* ctx, idcu_NodeInfo* nodes, size_t max_nodes, size_t* actual_count);

#ifdef __cplusplus
}
#endif

#endif
