#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "idcu/server_monitor/server_monitor.h"
#include "idcu/http_server/http_server.h"
#include "idcu/log/log.h"

static idcu_ServerMonitor g_monitor;
static idcu_HttpServer g_http_server;
static int g_running = 1;

static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

static void metrics_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    char buffer[8192];
    size_t size;
    
    if (idcu_server_monitor_export_prometheus(&g_monitor, buffer, sizeof(buffer), &size) == IDCU_ERR_OK) {
        idcu_http_response_set_status(res, 200);
        idcu_http_response_set_header(res, "Content-Type", "text/plain; version=0.0.4");
        idcu_http_response_set_body(res, buffer, size);
    } else {
        idcu_http_response_set_status(res, 500);
        idcu_http_response_set_body(res, "Failed to export metrics", 24);
    }
}

static void json_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    char buffer[8192];
    size_t size;
    
    if (idcu_server_monitor_export_json(&g_monitor, buffer, sizeof(buffer), &size) == IDCU_ERR_OK) {
        idcu_http_response_set_status(res, 200);
        idcu_http_response_set_header(res, "Content-Type", "application/json");
        idcu_http_response_set_body(res, buffer, size);
    } else {
        idcu_http_response_set_status(res, 500);
        idcu_http_response_set_body(res, "{\"error\":\"Failed to export metrics\"}", 34);
    }
}

int main(int argc, char* argv[]) {
    printf("=== Prometheus Integration Example ===\n\n");
    
    idcu_log_init();
    idcu_server_monitor_init(&g_monitor);
    
    idcu_http_server_init(&g_http_server);
    idcu_http_server_set_port(&g_http_server, 9090);
    
    idcu_http_server_add_route(&g_http_server, "/metrics", metrics_handler, NULL);
    idcu_http_server_add_route(&g_http_server, "/json", json_handler, NULL);
    
    if (idcu_http_server_start(&g_http_server) != IDCU_ERR_OK) {
        printf("Failed to start HTTP server\n");
        return 1;
    }
    
    printf("Prometheus exporter started on http://localhost:9090/metrics\n");
    printf("JSON endpoint available on http://localhost:9090/json\n");
    printf("Press Ctrl+C to stop.\n\n");
    
    while (g_running) {
        idcu_http_server_process(&g_http_server);
        sleep_ms(100);
    }
    
    idcu_http_server_stop(&g_http_server);
    idcu_http_server_destroy(&g_http_server);
    idcu_server_monitor_destroy(&g_monitor);
    idcu_log_destroy();
    
    printf("\nExample completed.\n");
    return 0;
}
