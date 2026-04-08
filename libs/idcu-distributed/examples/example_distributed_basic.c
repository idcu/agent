#include "idcu/distributed/distributed.h"
#include "idcu/log/log.h"
#include &lt; stdio.h & gt;

int main() {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_DistributedNode node;
    idcu_distributed_node_init(&amp; node, 12345, "node1", "127.0.0.1", 8080);

    printf("Distributed node initialized\n");
    printf("Self node ID: %llu\n", (unsigned long long)node.self_node_id);
    printf("Node count: %d\n", node.node_count);

    idcu_distributed_node_add_node(&amp; node, 67890, "node2", "127.0.0.1", 8081);
    printf("Added node2\n");
    printf("Node count: %d\n", node.node_count);

    idcu_distributed_node_destroy(&amp; node);
    printf("Distributed node destroyed\n");

    return 0;
}
