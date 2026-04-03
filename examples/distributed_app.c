#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "log.h"
#include "distributed_node.h"
#include "node_discovery.h"
#include "plugin_ecosystem.h"
#include "health_check.h"
#include "metrics.h"
#include "config.h"

static volatile int g_running = 1;

void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
    printf("\nShutting down...\n");
}

void on_node_discovered(uint64_t node_id, const char* name, const char* address, uint16_t port, void* user_data) {
    (void)user_data;
    log_info("Discovered node: %llu (%s) at %s:%u", 
             (unsigned long long)node_id, name, address, port);
}

void on_node_lost(uint64_t node_id, void* user_data) {
    (void)user_data;
    log_info("Lost node: %llu", (unsigned long long)node_id);
}

void health_callback(uint32_t module_id, HealthStatus old_status, HealthStatus new_status, void* user_data) {
    (void)user_data;
    log_info("Module %u health changed: %d -> %d", module_id, old_status, new_status);
}

int main(int argc, char* argv[]) {
    const char* config_file = "config/agent.cfg";
    if (argc > 1) {
        config_file = argv[1];
    }
    
    log_init(NULL, LOG_INFO);
    log_info("========================================");
    log_info("IDCU Agent - Distributed Application Demo");
    log_info("========================================");
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    log_info("Step 1: Initializing configuration...");
    if (config_init(config_file) != ERR_OK) {
        log_error("Failed to load config from %s", config_file);
        log_shutdown();
        return 1;
    }
    log_info("Configuration loaded successfully");
    
    log_info("\nStep 2: Initializing distributed node...");
    DistributedNode dist_node;
    const char* node_name = config_get_string("node", "name", "demo-node");
    const char* node_addr = config_get_string("node", "address", "127.0.0.1");
    uint16_t node_port = (uint16_t)config_get_int("node", "port", 8080);
    uint64_t node_id = (uint64_t)config_get_int("node", "id", 1);
    
    if (distributed_node_init(&dist_node, node_id, node_name, node_addr, node_port) != ERR_OK) {
        log_error("Failed to initialize distributed node");
        config_shutdown();
        log_shutdown();
        return 1;
    }
    log_info("Distributed node initialized: %s (%llu) at %s:%u", 
             node_name, (unsigned long long)node_id, node_addr, node_port);
    
    log_info("\nStep 3: Initializing health monitor...");
    HealthMonitor health_monitor;
    health_monitor_init(&health_monitor);
    health_set_callback(&health_monitor, health_callback, NULL);
    health_register_module(&health_monitor, 1);
    log_info("Health monitor initialized");
    
    log_info("\nStep 4: Initializing metrics collector...");
    MetricsCollector metrics;
    metrics_init(&metrics);
    metrics_register(&metrics, "requests_total", "Total requests", METRIC_COUNTER);
    metrics_register(&metrics, "active_connections", "Active connections", METRIC_GAUGE);
    metrics_register(&metrics, "latency_ms", "Request latency", METRIC_HISTOGRAM);
    log_info("Metrics collector initialized");
    
    log_info("\nStep 5: Initializing plugin ecosystem...");
    PluginEcosystem plugin_eco;
    plugin_ecosystem_init(&plugin_eco);
    plugin_ecosystem_add_plugin(&plugin_eco, "example-plugin", "1.0.0", "An example plugin");
    plugin_ecosystem_add_plugin(&plugin_eco, "data-collector", "2.1.0", "Data collection plugin");
    plugin_ecosystem_install_plugin(&plugin_eco, "example-plugin");
    plugin_ecosystem_activate_plugin(&plugin_eco, "example-plugin");
    log_info("Plugin ecosystem initialized with %u plugins", plugin_eco.plugin_count);
    
    log_info("\nStep 6: Initializing node discovery...");
    NodeDiscovery node_disc;
    if (node_discovery_init(&node_disc, &dist_node) != ERR_OK) {
        log_error("Failed to initialize node discovery");
        plugin_ecosystem_destroy(&plugin_eco);
        metrics_destroy(&metrics);
        health_monitor_destroy(&health_monitor);
        distributed_node_destroy(&dist_node);
        config_shutdown();
        log_shutdown();
        return 1;
    }
    node_discovery_set_discovered_handler(&node_disc, on_node_discovered, NULL);
    node_discovery_set_lost_handler(&node_disc, on_node_lost, NULL);
    log_info("Node discovery initialized");
    
    log_info("\nStep 7: Starting services...");
    node_discovery_start(&node_disc);
    log_info("All services started successfully");
    
    log_info("\n========================================");
    log_info("Application running! Press Ctrl+C to stop");
    log_info("========================================\n");
    
    uint64_t counter = 0;
    while (g_running) {
        counter++;
        
        if (counter % 10 == 0) {
            health_update_heartbeat(&health_monitor, 1);
            metrics_inc(&metrics, "requests_total", 1);
            metrics_set(&metrics, "active_connections", (uint64_t)(dist_node.node_count - 1));
            node_discovery_broadcast_hello(&node_disc);
            node_discovery_process(&node_disc);
        }
        
        if (counter % 50 == 0) {
            char metrics_buffer[1024];
            metrics_export_text(&metrics, metrics_buffer, sizeof(metrics_buffer));
            log_info("\n--- Metrics Snapshot ---");
            log_info("%s", metrics_buffer);
            log_info("------------------------\n");
        }
        
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }
    
    log_info("\nShutting down services...");
    node_discovery_stop(&node_disc);
    node_discovery_destroy(&node_disc);
    plugin_ecosystem_destroy(&plugin_eco);
    metrics_destroy(&metrics);
    health_monitor_destroy(&health_monitor);
    distributed_node_destroy(&dist_node);
    config_shutdown();
    log_info("All services stopped");
    
    log_info("Application shutdown complete");
    log_shutdown();
    
    return 0;
}
