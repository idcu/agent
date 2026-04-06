#include "idcu/distributed/distributed.h"
#include "idcu/log/log.h"
#include <stdio.h>

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_DistributedNode node;
    int ret = idcu_distributed_node_init(&node, 12345, "test_node", "127.0.0.1", 8080);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: init failed\n");
        return 1;
    }
    
    if (node.self_node_id != 12345) {
        printf("Test failed: node_id mismatch\n");
        idcu_distributed_node_destroy(&node);
        return 1;
    }
    
    if (node.node_count != 1) {
        printf("Test failed: node_count should be 1\n");
        idcu_distributed_node_destroy(&node);
        return 1;
    }
    
    idcu_distributed_node_destroy(&node);
    
    printf("Test passed\n");
    return 0;
}
