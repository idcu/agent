#include "test/test_framework.h"
#include "distributed_node.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_distributed_node_init_destroy(void) {
    idcu_DistributedNode node;
    int ret = idcu_distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_distributed_node_init should succeed");
    IDCU_TEST_ASSERT(node.self_node_id == 1, "Self node ID should be 1");
    IDCU_TEST_ASSERT(node.node_count == 1, "Node count should be 1");
    IDCU_TEST_ASSERT(strcmp(node.nodes[0].name, "self-node") == 0, "Node name should match");
    IDCU_TEST_ASSERT(strcmp(node.nodes[0].address, "127.0.0.1") == 0, "Node address should match");
    IDCU_TEST_ASSERT(node.nodes[0].port == 8080, "Node port should be 8080");
    
    idcu_distributed_node_destroy(&node);
    IDCU_TEST_PASS();
}

static void test_distributed_node_add_remove(void) {
    idcu_DistributedNode node;
    idcu_distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    
    int ret = idcu_distributed_node_add_node(&node, 2, "node2", "192.168.1.2", 8081);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node 2 should succeed");
    IDCU_TEST_ASSERT(node.node_count == 2, "Node count should be 2");
    
    ret = idcu_distributed_node_add_node(&node, 3, "node3", "192.168.1.3", 8082);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node 3 should succeed");
    IDCU_TEST_ASSERT(node.node_count == 3, "Node count should be 3");
    
    ret = idcu_distributed_node_add_node(&node, 2, "duplicate", "192.168.1.4", 8083);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK, "Add duplicate node should fail");
    IDCU_TEST_ASSERT(node.node_count == 3, "Node count should remain 3");
    
    ret = idcu_distributed_node_remove_node(&node, 2);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove node 2 should succeed");
    IDCU_TEST_ASSERT(node.node_count == 2, "Node count should be 2");
    
    ret = idcu_distributed_node_remove_node(&node, 1);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK, "Remove self node should fail");
    IDCU_TEST_ASSERT(node.node_count == 2, "Node count should remain 2");
    
    idcu_distributed_node_destroy(&node);
    IDCU_TEST_PASS();
}

static void test_distributed_node_find(void) {
    idcu_DistributedNode node;
    idcu_distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    
    idcu_distributed_node_add_node(&node, 2, "node2", "192.168.1.2", 8081);
    idcu_distributed_node_add_node(&node, 3, "node3", "192.168.1.3", 8082);
    
    idcu_NodeInfo* info = idcu_distributed_node_find_node(&node, 2);
    IDCU_TEST_ASSERT(info != NULL, "Find node 2 should succeed");
    IDCU_TEST_ASSERT(strcmp(info->name, "node2") == 0, "Node name should be node2");
    
    info = idcu_distributed_node_find_node(&node, 3);
    IDCU_TEST_ASSERT(info != NULL, "Find node 3 should succeed");
    IDCU_TEST_ASSERT(strcmp(info->name, "node3") == 0, "Node name should be node3");
    
    info = idcu_distributed_node_find_node(&node, 99);
    IDCU_TEST_ASSERT(info == NULL, "Find non-existent node should return NULL");
    
    idcu_distributed_node_destroy(&node);
    IDCU_TEST_PASS();
}

static void test_distributed_node_status_update(void) {
    idcu_DistributedNode node;
    idcu_distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    
    idcu_distributed_node_add_node(&node, 2, "node2", "192.168.1.2", 8081);
    
    int ret = idcu_distributed_node_update_node_status(&node, 2, IDCU_NODE_STATUS_BUSY);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Update node status should succeed");
    
    idcu_NodeInfo* info = idcu_distributed_node_find_node(&node, 2);
    IDCU_TEST_ASSERT(info->status == IDCU_NODE_STATUS_BUSY, "Node status should be BUSY");
    
    ret = idcu_distributed_node_update_heartbeat(&node, 2);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Update heartbeat should succeed");
    
    idcu_distributed_node_destroy(&node);
    IDCU_TEST_PASS();
}

static void test_distributed_node_message_queue(void) {
    idcu_DistributedNode node;
    idcu_distributed_node_init(&node, 1, "self-node", "127.0.0.1", 8080);
    
    char payload[] = "test message";
    int ret = idcu_distributed_node_send_message(&node, 2, 100, payload, sizeof(payload));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Send message should succeed");
    IDCU_TEST_ASSERT(node.msg_count == 1, "Message count should be 1");
    
    idcu_NodeMessage msg;
    ret = idcu_distributed_node_recv_message(&node, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Receive message should succeed");
    IDCU_TEST_ASSERT(msg.type == 100, "Message type should be 100");
    IDCU_TEST_ASSERT(node.msg_count == 0, "Message count should be 0");
    
    idcu_distributed_node_destroy(&node);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Distributed Node Tests");
    
    idcu_test_suite_add_test(&g_suite, "distributed_node_init_destroy", test_distributed_node_init_destroy);
    idcu_test_suite_add_test(&g_suite, "distributed_node_add_remove", test_distributed_node_add_remove);
    idcu_test_suite_add_test(&g_suite, "distributed_node_find", test_distributed_node_find);
    idcu_test_suite_add_test(&g_suite, "distributed_node_status_update", test_distributed_node_status_update);
    idcu_test_suite_add_test(&g_suite, "distributed_node_message_queue", test_distributed_node_message_queue);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
