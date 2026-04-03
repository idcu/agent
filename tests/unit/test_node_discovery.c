#include "test/test_framework.h"
#include "network/node_discovery.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_node_discovery_init_destroy(void) {
    DistributedNode dist_node;
    distributed_node_init(&dist_node, 1, "test-node", "127.0.0.1", 8080);
    
    NodeDiscovery disc;
    int ret = node_discovery_init(&disc, &dist_node);
    TEST_ASSERT(ret == ERR_OK, "node_discovery_init should succeed");
    TEST_ASSERT(disc.dist_node == &dist_node, "Dist node should be set");
    
    node_discovery_destroy(&disc);
    distributed_node_destroy(&dist_node);
    TEST_PASS();
}

static void test_node_discovery_callbacks(void) {
    DistributedNode dist_node;
    distributed_node_init(&dist_node, 1, "test-node", "127.0.0.1", 8080);
    
    NodeDiscovery disc;
    node_discovery_init(&disc, &dist_node);
    
    int ret = node_discovery_set_discovered_handler(&disc, NULL, NULL);
    TEST_ASSERT(ret == ERR_OK, "Set discovered handler should succeed");
    
    ret = node_discovery_set_lost_handler(&disc, NULL, NULL);
    TEST_ASSERT(ret == ERR_OK, "Set lost handler should succeed");
    
    node_discovery_destroy(&disc);
    distributed_node_destroy(&dist_node);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Node Discovery Tests");
    
    test_suite_add_test(&g_suite, "node_discovery_init_destroy", test_node_discovery_init_destroy);
    test_suite_add_test(&g_suite, "node_discovery_callbacks", test_node_discovery_callbacks);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
