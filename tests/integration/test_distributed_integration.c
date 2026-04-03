#include "test/test_framework.h"
#include "network/distributed_node.h"
#include "network/node_discovery.h"
#include "network/network_layer.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#define msleep(ms) Sleep(ms)
#else
#include <unistd.h>
#define msleep(ms) usleep((ms) * 1000)
#endif

static idcu_TestSuite g_suite;

typedef struct {
    idcu_DistributedNode node;
    idcu_NodeDiscovery discovery;
    int discovered_count;
    int lost_count;
    int running;
} TestDistributedNode;

static void on_node_discovered(void* user_data, idcu_NodeInfo* node) {
    (void)user_data;
    (void)node;
}

static void on_node_lost(void* user_data, idcu_NodeInfo* node) {
    (void)user_data;
    (void)node;
}

static void test_distributed_two_node_communication(void) {
    idcu_DistributedNode node1, node2;
    int ret;

    ret = idcu_distributed_node_init(&node1, 1, "node1", "127.0.0.1", 9001);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Init node1 should succeed");

    ret = idcu_distributed_node_init(&node2, 2, "node2", "127.0.0.1", 9002);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Init node2 should succeed");

    ret = idcu_distributed_node_add_node(&node1, 2, "node2", "127.0.0.1", 9002);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node2 to node1 should succeed");

    ret = idcu_distributed_node_add_node(&node2, 1, "node1", "127.0.0.1", 9001);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node1 to node2 should succeed");

    char test_payload[] = "Hello from node1!";
    ret = idcu_distributed_node_send_message(&node1, 2, 1, test_payload, sizeof(test_payload));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Send message from node1 to node2 should succeed");

    idcu_NodeMessage msg;
    ret = idcu_distributed_node_recv_message(&node1, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Receive message should succeed (fallback queue)");
    IDCU_TEST_ASSERT(msg.type == 1, "Message type should be 1");
    IDCU_TEST_ASSERT(strcmp((char*)msg.payload, test_payload) == 0, "Payload should match");

    idcu_distributed_node_destroy(&node1);
    idcu_distributed_node_destroy(&node2);
    IDCU_TEST_PASS();
}

static void test_distributed_node_discovery_basic(void) {
    idcu_DistributedNode dist_node;
    idcu_NodeDiscovery disc;
    int ret;

    ret = idcu_distributed_node_init(&dist_node, 100, "discovery-test", "127.0.0.1", 9100);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Init distributed node should succeed");

    ret = idcu_node_discovery_init(&disc, &dist_node);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Init node discovery should succeed");

    ret = idcu_node_discovery_set_discovered_handler(&disc, on_node_discovered, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set discovered handler should succeed");

    ret = idcu_node_discovery_set_lost_handler(&disc, on_node_lost, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set lost handler should succeed");

    ret = idcu_node_discovery_start(&disc);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Start discovery should succeed");

    msleep(100);

    ret = idcu_node_discovery_poll(&disc);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Poll discovery should succeed");

    ret = idcu_node_discovery_stop(&disc);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Stop discovery should succeed");

    idcu_node_discovery_destroy(&disc);
    idcu_distributed_node_destroy(&dist_node);
    IDCU_TEST_PASS();
}

static void test_distributed_node_load_balancing(void) {
    idcu_DistributedNode node;
    int ret;
    uint64_t selected;

    ret = idcu_distributed_node_init(&node, 1, "load-balancer", "127.0.0.1", 9200);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Init load balancer node should succeed");

    ret = idcu_distributed_node_add_node(&node, 2, "node2", "192.168.1.2", 9201);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node2 should succeed");

    ret = idcu_distributed_node_add_node(&node, 3, "node3", "192.168.1.3", 9202);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node3 should succeed");

    idcu_NodeInfo* node2 = idcu_distributed_node_find_node(&node, 2);
    idcu_NodeInfo* node3 = idcu_distributed_node_find_node(&node, 3);
    IDCU_TEST_ASSERT(node2 != NULL && node3 != NULL, "Nodes should be found");

    node2->status = IDCU_NODE_STATUS_ONLINE;
    node2->load = 50;

    node3->status = IDCU_NODE_STATUS_ONLINE;
    node3->load = 10;

    selected = idcu_distributed_node_select_node_by_load(&node);
    IDCU_TEST_ASSERT(selected == 3, "Should select node3 with lower load");

    node3->load = 100;
    selected = idcu_distributed_node_select_node_by_load(&node);
    IDCU_TEST_ASSERT(selected == 2, "Should select node2 now that node3 is busier");

    node2->status = IDCU_NODE_STATUS_OFFLINE;
    selected = idcu_distributed_node_select_node_by_load(&node);
    IDCU_TEST_ASSERT(selected == 3, "Should select node3 when node2 is offline");

    idcu_distributed_node_destroy(&node);
    IDCU_TEST_PASS();
}

static void test_distributed_message_queue_full(void) {
    idcu_DistributedNode node;
    int ret;

    ret = idcu_distributed_node_init(&node, 1, "queue-test", "127.0.0.1", 9300);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Init node should succeed");

    char payload[] = "test";
    for (int i = 0; i < IDCU_MAX_MESSAGES; i++) {
        ret = idcu_distributed_node_send_message(&node, 999, i, payload, sizeof(payload));
        IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Send message should succeed");
    }

    ret = idcu_distributed_node_send_message(&node, 999, 999, payload, sizeof(payload));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_QUEUE_FULL, "Queue should be full after IDCU_MAX_MESSAGES");

    idcu_NodeMessage msg;
    for (int i = 0; i < IDCU_MAX_MESSAGES; i++) {
        ret = idcu_distributed_node_recv_message(&node, &msg);
        IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Receive message should succeed");
    }
    IDCU_TEST_ASSERT(msg.type == (uint32_t)(IDCU_MAX_MESSAGES - 1), "Last message type should match");

    ret = idcu_distributed_node_recv_message(&node, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_QUEUE_EMPTY, "Queue should be empty after receiving all");

    idcu_distributed_node_destroy(&node);
    IDCU_TEST_PASS();
}

static void test_distributed_node_lifecycle(void) {
    idcu_DistributedNode node;
    int ret;

    ret = idcu_distributed_node_init(&node, 1, "lifecycle-test", "127.0.0.1", 9400);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Init should succeed");
    IDCU_TEST_ASSERT(node.self_node_id == 1, "Node ID should be 1");
    IDCU_TEST_ASSERT(node.node_count == 1, "Node count should be 1");

    ret = idcu_distributed_node_add_node(&node, 2, "node2", "192.168.1.2", 9402);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node2 should succeed");
    ret = idcu_distributed_node_add_node(&node, 3, "node3", "192.168.1.3", 9403);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node3 should succeed");
    ret = idcu_distributed_node_add_node(&node, 4, "node4", "192.168.1.4", 9404);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node4 should succeed");
    ret = idcu_distributed_node_add_node(&node, 5, "node5", "192.168.1.5", 9405);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node5 should succeed");
    ret = idcu_distributed_node_add_node(&node, 6, "node6", "192.168.1.6", 9406);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add node6 should succeed");
    
    IDCU_TEST_ASSERT(node.node_count == 6, "Node count should be 6 after adding 5 nodes");

    ret = idcu_distributed_node_remove_node(&node, 2);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove node2 should succeed");
    ret = idcu_distributed_node_remove_node(&node, 3);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove node3 should succeed");
    ret = idcu_distributed_node_remove_node(&node, 4);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove node4 should succeed");
    ret = idcu_distributed_node_remove_node(&node, 5);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove node5 should succeed");
    
    IDCU_TEST_ASSERT(node.node_count == 2, "Node count should be 2 after removal");

    idcu_NodeInfo* info = idcu_distributed_node_find_node(&node, 6);
    IDCU_TEST_ASSERT(info != NULL, "Node6 should still exist");

    idcu_distributed_node_destroy(&node);
    IDCU_TEST_PASS();
}

static void test_distributed_message_serialization(void) {
    int ret;
    idcu_NodeMessage original;

    memset(&original, 0, sizeof(original));
    original.magic = IDCU_MESSAGE_MAGIC;
    original.version = IDCU_MESSAGE_VERSION;
    original.from_node = 12345;
    original.to_node = 67890;
    original.type = 0xABCDEF;
    original.payload_size = 16;
    memcpy(original.payload, "test payload data", 16);

    idcu_DistributedNode node;
    idcu_distributed_node_init(&node, 1, "test", "127.0.0.1", 0);

    ret = idcu_distributed_node_send_message(&node, 2, original.type, original.payload, original.payload_size);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Send message should succeed");

    idcu_NodeMessage received;
    ret = idcu_distributed_node_recv_message(&node, &received);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Receive message should succeed");
    IDCU_TEST_ASSERT(received.from_node == 1, "From node should be 1");
    IDCU_TEST_ASSERT(received.to_node == 2, "To node should be 2");
    IDCU_TEST_ASSERT(received.type == original.type, "Type should match");
    IDCU_TEST_ASSERT(received.payload_size == original.payload_size, "Payload size should match");
    IDCU_TEST_ASSERT(memcmp(received.payload, original.payload, original.payload_size) == 0, "Payload should match");

    idcu_distributed_node_destroy(&node);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_test_suite_init(&g_suite, "Distributed Integration Tests");

    idcu_test_suite_add_test(&g_suite, "distributed_two_node_communication", test_distributed_two_node_communication);
    idcu_test_suite_add_test(&g_suite, "distributed_node_discovery_basic", test_distributed_node_discovery_basic);
    idcu_test_suite_add_test(&g_suite, "distributed_node_load_balancing", test_distributed_node_load_balancing);
    idcu_test_suite_add_test(&g_suite, "distributed_message_queue_full", test_distributed_message_queue_full);
    idcu_test_suite_add_test(&g_suite, "distributed_node_lifecycle", test_distributed_node_lifecycle);
    idcu_test_suite_add_test(&g_suite, "distributed_message_serialization", test_distributed_message_serialization);

    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);

    int failures = idcu_test_suite_get_failures(&g_suite);

    return failures > 0 ? 1 : 0;
}
