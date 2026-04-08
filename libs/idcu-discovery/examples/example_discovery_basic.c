#include "idcu/discovery/discovery.h"
#include "idcu/log/log.h"
#include &lt; stdio.h & gt;

int main() {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_DistributedNode dist_node;
    idcu_distributed_node_init(&amp; dist_node, 12345, "node1", "127.0.0.1", 8080);

    idcu_NodeDiscovery disc;
    idcu_node_discovery_init(&amp; disc, &amp; dist_node);

    printf("Node discovery initialized\n");

    idcu_node_discovery_start(&amp; disc);
    printf("Node discovery started\n");

    idcu_node_discovery_stop(&amp; disc);
    printf("Node discovery stopped\n");

    idcu_node_discovery_destroy(&amp; disc);
    idcu_distributed_node_destroy(&amp; dist_node);

    return 0;
}
