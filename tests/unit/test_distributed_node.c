#include "test/test_framework.h"
#include "network/distributed_node.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_distributed_node_init_destroy(void) {
    DistributedNode node;
    int ret = distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    TEST_ASSERT(ret == ERR_OK, "distributed_node_init should succeed");
    TEST_ASSERT(node.self_node_id == 1, "Self node ID should be 1");
    TEST_ASSERT(node.node_count == 1, "Node count should be 1");
    TEST_ASSERT(strcmp(node.nodes[0].name, "self-node") == 0, "Node name should match");
    TEST_ASSERT(strcmp(node.nodes[0].address, "127.0.0.1") == 0, "Node address should match");
    TEST_ASSERT(node.nodes[0].port == 8080, "Node port should be 8080");
    
    distributed_node_destroy(&node);
    TEST_PASS();
}

static void test_distributed_node_add_remove(void) {
    DistributedNode node;
    distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    
    int ret = distributed_node_add_node(&node, 2, "node2", "192.168.1.2", 8081);
    TEST_ASSERT(ret == ERR_OK, "Add node 2 should succeed");
    TEST_ASSERT(node.node_count == 2, "Node count should be 2");
    
    ret = distributed_node_add_node(&node, 3, "node3", "192.168.1.3", 8082);
    TEST_ASSERT(ret == ERR_OK, "Add node 3 should succeed");
    TEST_ASSERT(node.node_count == 3, "Node count should be 3");
    
    ret = distributed_node_add_node(&node, 2, "duplicate", "192.168.1.4", 8083);
    TEST_ASSERT(ret != ERR_OK, "Add duplicate node should fail");
    TEST_ASSERT(node.node_count == 3, "Node count should remain 3");
    
    ret = distributed_node_remove_node(&node, 2);
    TEST_ASSERT(ret == ERR_OK, "Remove node 2 should succeed");
    TEST_ASSERT(node.node_count == 2, "Node count should be 2");
    
    ret = distributed_node_remove_node(&node, 1);
    TEST_ASSERT(ret != ERR_OK, "Remove self node should fail");
    TEST_ASSERT(node.node_count == 2, "Node count should remain 2");
    
    distributed_node_destroy(&node);
    TEST_PASS();
}

static void test_distributed_node_find(void) {
    DistributedNode node;
    distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    
    distributed_node_add_node(&node, 2, "node2", "192.168.1.2", 8081);
    distributed_node_add_node(&node, 3, "node3", "192.168.1.3", 8082);
    
    NodeInfo* info = distributed_node_find_node(&node, 2);
    TEST_ASSERT(info != NULL, "Find node 2 should succeed");
    TEST_ASSERT(strcmp(info->name, "node2") == 0, "Node name should be node2");
    
    info = distributed_node_find_node(&node, 3);
    TEST_ASSERT(info != NULL, "Find node 3 should succeed");
    TEST_ASSERT(strcmp(info->name, "node3") == 0, "Node name should be node3");
    
    info = distributed_node_find_node(&node, 99);
    TEST_ASSERT(info == NULL, "Find non-existent node should return NULL");
    
    distributed_node_destroy(&node);
    TEST_PASS();
}

static void test_distributed_node_status_update(void) {
    DistributedNode node;
    distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    
    distributed_node_add_node(&node, 2, "node2", "192.168.1.2", 8081);
    
    int ret = distributed_node_update_node_status(&node, 2, NODE_STATUS_BUSY);
    TEST_ASSERT(ret == ERR_OK, "Update node status should succeed");
    
    NodeInfo* info = distributed_node_find_node(&node, 2);
    TEST_ASSERT(info->status == NODE_STATUS_BUSY, "Node status should be BUSY");
    
    ret = distributed_node_update_heartbeat(&node, 2);
    TEST_ASSERT(ret == ERR_OK, "Update heartbeat should succeed");
    
    distributed_node_destroy(&node);
    TEST_PASS();
}

static void test_distributed_node_message_queue(void) {
    DistributedNode node;
    distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    
    char payload[] = "test message";
    int ret = distributed_node_send_message(&node, 2, 100, payload, sizeof(payload));
    TEST_ASSERT(ret == ERR_OK, "Send message should succeed");
    TEST_ASSERT(node.msg_count == 1, "Message count should be 1");
    
    NodeMessage msg;
    ret = distributed_node_recv_message(&node, &msg);
    TEST_ASSERT(ret == ERR_OK, "Receive message should succeed");
    TEST_ASSERT(msg.type == 100, "Message type should be 100");
    TEST_ASSERT(node.msg_count == 0, "Message count should be 0");
    
    distributed_node_destroy(&node);
    TEST_PASS();
}

static void test_distributed_node_load_balance(void) {
    DistributedNode node;
    distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    
    distributed_node_add_node(&node, 2, "node2", "192.168.1.2", 8081);
    distributed_node_add_node(&node, 3, "node3", "192.168.1.3", 8082);
    
    NodeInfo* node2 = distributed_node_find_node(&node, 2);
    node2->load = 100;
    node2->status = NODE_STATUS_ONLINE;
    
    NodeInfo* node3 = distributed_node_find_node(&node, 3);
    node3->load = 50;
    node3->status = NODE_STATUS_ONLINE;
    
    uint64_t selected = distributed_node_select_node_by_load(&node);
    TEST_ASSERT(selected == 3, "Should select node with least load");
    
    distributed_node_destroy(&node);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Distributed Node Tests");
    
    test_suite_add_test(&g_suite, "distributed_node_init_destroy", test_distributed_node_init_destroy);
    test_suite_add_test(&g_suite, "distributed_node_add_remove", test_distributed_node_add_remove);
    test_suite_add_test(&g_suite, "distributed_node_find", test_distributed_node_find);
    test_suite_add_test(&g_suite, "distributed_node_status_update", test_distributed_node_status_update);
    test_suite_add_test(&g_suite, "distributed_node_message_queue", test_distributed_node_message_queue);
    test_suite_add_test(&g_suite, "distributed_node_load_balance", test_distributed_node_load_balance);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
