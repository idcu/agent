#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "idcu/server_monitor/server_monitor.h"
#include "idcu/http_client/http_client.h"
#include "idcu/json/json.h"
#include "idcu/log/log.h"

static idcu_ServerMonitor g_monitor;
static idcu_HttpClient g_http_client;
static int g_running = 1;
static const char* g_elasticsearch_url = "http://localhost:9200";
static const char* g_index_name = "idcu-metrics";

static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

static int send_to_elasticsearch(const char* json_data, size_t json_size) {
    char url[512];
    snprintf(url, sizeof(url), "%s/%s/_doc", g_elasticsearch_url, g_index_name);
    
    idcu_HttpRequest req;
    idcu_http_request_init(&req);
    idcu_http_request_set_method(&req, "POST");
    idcu_http_request_set_url(&req, url);
    idcu_http_request_set_header(&req, "Content-Type", "application/json");
    idcu_http_request_set_body(&req, json_data, json_size);
    
    idcu_HttpResponse res;
    idcu_http_response_init(&res);
    
    int ret = idcu_http_client_send(&g_http_client, &req, &res);
    
    if (ret == IDCU_ERR_OK && res.status_code >= 200 && res.status_code < 300) {
        printf("Successfully sent to Elasticsearch\n");
    } else {
        printf("Failed to send to Elasticsearch, status: %d\n", res.status_code);
    }
    
    idcu_http_request_destroy(&req);
    idcu_http_response_destroy(&res);
    
    return ret;
}

static void collect_and_send(void) {
    char buffer[16384];
    size_t size;
    
    if (idcu_server_monitor_export_json(&g_monitor, buffer, sizeof(buffer), &size) != IDCU_ERR_OK) {
        printf("Failed to export metrics\n");
        return;
    }
    
    send_to_elasticsearch(buffer, size);
}

int main(int argc, char* argv[]) {
    printf("=== ELK Integration Example ===\n\n");
    
    if (argc > 1) {
        g_elasticsearch_url = argv[1];
    }
    if (argc > 2) {
        g_index_name = argv[2];
    }
    
    printf("Elasticsearch URL: %s\n", g_elasticsearch_url);
    printf("Index name: %s\n", g_index_name);
    printf("\n");
    
    idcu_log_init();
    idcu_server_monitor_init(&g_monitor);
    idcu_http_client_init(&g_http_client);
    
    printf("Starting metrics collection. Sending every 30 seconds.\n");
    printf("Press Ctrl+C to stop.\n\n");
    
    int iteration = 0;
    while (g_running) {
        if (iteration % 300 == 0) {
            collect_and_send();
        }
        sleep_ms(100);
        iteration++;
    }
    
    idcu_http_client_destroy(&g_http_client);
    idcu_server_monitor_destroy(&g_monitor);
    idcu_log_destroy();
    
    printf("\nExample completed.\n");
    return 0;
}
