/**
 * @file distributed_node_example.c
 * @brief 分布式节点通信示例
 *
 * 本示例演示如何使用IDCU的分布式节点功能，
 * 实现多个节点之间的发现、通信和数据同步。
 */

#include "idcu/distributed/distributed.h"
#include "idcu/discovery/discovery.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

typedef struct {
    uint64_t node_id;
    char node_name[64];
    char ip_address[64];
    uint16_t port;
    bool is_connected;
} NodeInfo;

static void print_node_list(idcu_DistributedNode* node) {
    printf("\n=== Connected Nodes ===\n");
    printf("Self: Node %llu (%s) at %s:%u\n", 
           (unsigned long long)node->self_node_id,
           node->self_node_name,
           node->self_ip,
           node->self_port);
    
    for (int i = 0; i < node->node_count; i++) {
        printf("Peer %d: Node %llu (%s) at %s:%u\n", 
               i,
               (unsigned long long)node->nodes[i].node_id,
               node->nodes[i].node_name,
               node->nodes[i].ip_address,
               node->nodes[i].port);
    }
    printf("=======================\n\n");
}

static int send_message_to_node(idcu_DistributedNode* node, uint64_t target_node_id, const char* message) {
    printf("Sending message to Node %llu: %s\n", (unsigned long long)target_node_id, message);
    
    idcu_DistributedMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.source_node_id = node->self_node_id;
    msg.target_node_id = target_node_id;
    msg.message_type = IDCU_DISTRIBUTED_MSG_TYPE_DATA;
    msg.data_length = (uint32_t)strlen(message);
    memcpy(msg.data, message, msg.data_length);
    
    int ret = idcu_distributed_send_message(node, &msg);
    if (ret == IDCU_ERR_OK) {
        printf("Message sent successfully!\n");
    } else {
        printf("Failed to send message: %d\n", ret);
    }
    
    return ret;
}

static int broadcast_message(idcu_DistributedNode* node, const char* message) {
    printf("Broadcasting message: %s\n", message);
    
    idcu_DistributedMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.source_node_id = node->self_node_id;
    msg.target_node_id = 0;
    msg.message_type = IDCU_DISTRIBUTED_MSG_TYPE_BROADCAST;
    msg.data_length = (uint32_t)strlen(message);
    memcpy(msg.data, message, msg.data_length);
    
    int ret = idcu_distributed_broadcast_message(node, &msg);
    if (ret == IDCU_ERR_OK) {
        printf("Broadcast sent successfully!\n");
    } else {
        printf("Failed to broadcast: %d\n", ret);
    }
    
    return ret;
}

static int demonstrate_distributed_node(const char* node_name, uint16_t port) {
    printf("====================================\n");
    printf("Distributed Node Example\n");
    printf("====================================\n\n");
    
    idcu_DistributedNode node;
    int ret = idcu_distributed_node_init(&node, port, node_name, "127.0.0.1", port);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize distributed node: %d\n", ret);
        return 1;
    }
    
    printf("Distributed node initialized\n");
    printf("Node ID: %llu\n", (unsigned long long)node.self_node_id);
    printf("Node Name: %s\n", node.self_node_name);
    printf("Listening on: %s:%u\n\n", node.self_ip, node.self_port);
    
    printf("Step 1: Add some peer nodes (simulation)...\n");
    
    ret = idcu_distributed_node_add_node(&node, 1001, "worker-node-1", "192.168.1.101", 8001);
    if (ret == IDCU_ERR_OK) {
        printf("Added worker-node-1\n");
    }
    
    ret = idcu_distributed_node_add_node(&node, 1002, "worker-node-2", "192.168.1.102", 8002);
    if (ret == IDCU_ERR_OK) {
        printf("Added worker-node-2\n");
    }
    
    ret = idcu_distributed_node_add_node(&node, 1003, "master-node", "192.168.1.100", 8000);
    if (ret == IDCU_ERR_OK) {
        printf("Added master-node\n");
    }
    
    print_node_list(&node);
    
    printf("Step 2: Send a message to a specific node...\n");
    send_message_to_node(&node, 1001, "Hello from coordinator!");
    
    printf("\nStep 3: Broadcast a message to all nodes...\n");
    broadcast_message(&node, "System status update: All systems go!");
    
    printf("\nStep 4: Remove a node...\n");
    ret = idcu_distributed_node_remove_node(&node, 1002);
    if (ret == IDCU_ERR_OK) {
        printf("Removed worker-node-2\n");
    }
    
    print_node_list(&node);
    
    printf("Step 5: Simulate node discovery...\n");
    printf("  - Node discovery would scan the network for other IDCU nodes\n");
    printf("  - Discovered nodes would be automatically added to the peer list\n");
    printf("  - Heartbeat messages ensure nodes are still alive\n\n");
    
    idcu_DiscoveryService discovery;
    ret = idcu_discovery_init(&discovery, port + 100);
    if (ret == IDCU_ERR_OK) {
        printf("Discovery service initialized on port %u\n", port + 100);
        printf("  - Multicast address: 239.255.0.1\n");
        printf("  - Announcement interval: 5 seconds\n");
        printf("  - Node timeout: 15 seconds\n");
        idcu_discovery_destroy(&discovery);
    }
    printf("\n");
    
    idcu_distributed_node_destroy(&node);
    
    printf("Distributed node cleaned up\n\n");
    
    return 0;
}

int main(int argc, char* argv[]) {
    const char* node_name = "coordinator";
    uint16_t port = 9000;
    
    if (argc > 1) {
        node_name = argv[1];
    }
    if (argc > 2) {
        port = (uint16_t)atoi(argv[2]);
    }
    
    printf("====================================\n");
    printf("Distributed Node Communication Example\n");
    printf("====================================\n\n");
    
    printf("This example demonstrates distributed node communication.\n\n");
    printf("Usage:\n");
    printf("  %s [node_name] [port]\n\n", argv[0]);
    
    printf("Key features demonstrated:\n");
    printf("  - Node initialization and identification\n");
    printf("  - Peer node management (add/remove)\n");
    printf("  - Point-to-point messaging\n");
    printf("  - Broadcast messaging\n");
    printf("  - Node discovery service\n");
    printf("  - Heartbeat and health checking\n\n");
    
    printf("In a real deployment:\n");
    printf("  1. Start multiple instances on different machines/ports\n");
    printf("  2. Nodes will discover each other automatically\n");
    printf("  3. Send messages between nodes for coordination\n");
    printf("  4. Monitor node health via heartbeats\n\n");
    
    return demonstrate_distributed_node(node_name, port);
}
