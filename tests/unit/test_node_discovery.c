#include "test/test_framework.h"
#include "network/node_discovery.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_node_discovery_init_destroy(void) {
    idcu_DistributedNode dist_node;
    idcu_distributed_node_init(&dist_node, 1, "test-node", "127.0.0.1", 8080);
    
    idcu_NodeDiscovery disc;
    int ret = idcu_node_discovery_init(&disc, &dist_node);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_node_discovery_init should succeed");
    IDCU_TEST_ASSERT(disc.dist_node == &dist_node, "Dist node should be set");
    
    idcu_node_discovery_destroy(&disc);
    idcu_distributed_node_destroy(&dist_node);
    IDCU_TEST_PASS();
}

static void test_node_discovery_callbacks(void) {
    idcu_DistributedNode dist_node;
    idcu_distributed_node_init(&dist_node, 1, "test-node", "127.0.0.1", 8080);
    
    idcu_NodeDiscovery disc;
    idcu_node_discovery_init(&disc, &dist_node);
    
    int ret = idcu_node_discovery_set_discovered_handler(&disc, NULL, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set discovered handler should succeed");
    
    ret = idcu_node_discovery_set_lost_handler(&disc, NULL, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set lost handler should succeed");
    
    idcu_node_discovery_destroy(&disc);
    idcu_distributed_node_destroy(&dist_node);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Node Discovery Tests");
    
    idcu_test_suite_add_test(&g_suite, "node_discovery_init_destroy", test_node_discovery_init_destroy);
    idcu_test_suite_add_test(&g_suite, "node_discovery_callbacks", test_node_discovery_callbacks);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
