#include "idcu/discovery/discovery.h"
#include "idcu/log/log.h"
#include <stdio.h>

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_DistributedNode dist_node;
    int ret = idcu_distributed_node_init(&dist_node, 12345, "test_node", "127.0.0.1", 8080);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: distributed node init failed\n");
        return 1;
    }

    idcu_NodeDiscovery disc;
    ret = idcu_node_discovery_init(&disc, &dist_node);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: discovery init failed\n");
        idcu_distributed_node_destroy(&dist_node);
        return 1;
    }

    idcu_node_discovery_destroy(&disc);
    idcu_distributed_node_destroy(&dist_node);

    printf("Test passed\n");
    return 0;
}
