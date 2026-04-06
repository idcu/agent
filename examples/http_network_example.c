/**
 * @file http_network_example.c
 * @brief HTTP客户端和服务端网络交互示例
 *
 * 本示例演示如何使用IDCU的HTTP服务端和客户端库进行网络交互。
 */

#include "idcu/http_server/http_server.h"
#include "idcu/http_client/http_client.h"
#include "idcu/json/json.h"
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

#define SERVER_PORT 8080
#define MAX_RESPONSE_SIZE 4096

typedef struct {
    int request_count;
    double total_processing_time_ms;
} HttpServerState;

static HttpServerState g_server_state = {0, 0.0};

int hello_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    g_server_state.request_count++;
    
    idcu_http_response_set_body(response, "Hello, IDCU HTTP Server!", 23);
    idcu_http_response_set_header(response, "Content-Type", "text/plain");
    return IDCU_ERR_OK;
}

int status_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    
    char json_body[512];
    snprintf(json_body, sizeof(json_body), 
             "{\"status\":\"ok\",\"requests\":%d,\"uptime\":\"running\"}", 
             g_server_state.request_count);
    
    idcu_http_response_set_json_body(response, json_body);
    return IDCU_ERR_OK;
}

int echo_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)user_data;
    g_server_state.request_count++;
    
    if (request->body_length > 0) {
        idcu_http_response_set_body(response, request->body, request->body_length);
    } else {
        idcu_http_response_set_body(response, "No body received", 16);
    }
    idcu_http_response_set_header(response, "Content-Type", "text/plain");
    return IDCU_ERR_OK;
}

int api_data_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    g_server_state.request_count++;
    
    idcu_JsonObject* root = idcu_json_object_create();
    idcu_json_object_add_string(root, "message", "Success");
    idcu_json_object_add_number(root, "status", 200);
    idcu_json_object_add_number(root, "timestamp", (double)time(NULL));
    
    idcu_JsonArray* data = idcu_json_array_create();
    idcu_json_array_add_string(data, "item1");
    idcu_json_array_add_string(data, "item2");
    idcu_json_array_add_string(data, "item3");
    idcu_json_object_add_array(root, "data", data);
    
    char* json_str = idcu_json_serialize(root);
    idcu_http_response_set_json_body(response, json_str);
    
    free(json_str);
    idcu_json_object_destroy(root);
    
    return IDCU_ERR_OK;
}

static int run_http_server(void) {
    printf("Starting HTTP server on port %d...\n", SERVER_PORT);
    printf("Available routes:\n");
    printf("  GET  /          - Hello World\n");
    printf("  GET  /status    - Server status (JSON)\n");
    printf("  POST /echo      - Echo request body\n");
    printf("  GET  /api/data  - API data (JSON)\n");
    printf("\nPress Ctrl+C to stop the server\n\n");
    
    idcu_HttpServer server;
    int ret = idcu_http_server_init(&server, "0.0.0.0", SERVER_PORT);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize server: %d\n", ret);
        return 1;
    }
    
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/", hello_handler, NULL);
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/status", status_handler, NULL);
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_POST, "/echo", echo_handler, NULL);
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/api/data", api_data_handler, NULL);
    
    ret = idcu_http_server_start(&server);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to start server: %d\n", ret);
        idcu_http_server_destroy(&server);
        return 1;
    }
    
    printf("Server started successfully!\n");
    
    while (1) {
        idcu_http_server_poll(&server, 100);
    }
    
    idcu_http_server_destroy(&server);
    return 0;
}

static int run_http_client(void) {
    printf("Running HTTP client example...\n\n");
    
    idcu_HttpClient client;
    int ret = idcu_http_client_init(&client);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize HTTP client: %d\n", ret);
        return 1;
    }
    
    char response_buffer[MAX_RESPONSE_SIZE];
    idcu_HttpResponse* response;
    
    printf("=== Test 1: GET /\n");
    ret = idcu_http_client_get(&client, "http://localhost:8080/", response_buffer, sizeof(response_buffer), &response);
    if (ret == IDCU_ERR_OK) {
        printf("Status: %d\n", response->status_code);
        printf("Body: %.*s\n\n", (int)response->body_length, response->body);
    } else {
        printf("Request failed: %d\n\n", ret);
    }
    
    printf("=== Test 2: GET /status\n");
    ret = idcu_http_client_get(&client, "http://localhost:8080/status", response_buffer, sizeof(response_buffer), &response);
    if (ret == IDCU_ERR_OK) {
        printf("Status: %d\n", response->status_code);
        printf("Body: %.*s\n\n", (int)response->body_length, response->body);
    } else {
        printf("Request failed: %d\n\n", ret);
    }
    
    printf("=== Test 3: POST /echo\n");
    const char* post_data = "{\"name\":\"IDCU\",\"version\":\"1.0.0\"}";
    ret = idcu_http_client_post(&client, "http://localhost:8080/echo", post_data, strlen(post_data), 
                                  response_buffer, sizeof(response_buffer), &response);
    if (ret == IDCU_ERR_OK) {
        printf("Status: %d\n", response->status_code);
        printf("Body: %.*s\n\n", (int)response->body_length, response->body);
    } else {
        printf("Request failed: %d\n\n", ret);
    }
    
    printf("=== Test 4: GET /api/data\n");
    ret = idcu_http_client_get(&client, "http://localhost:8080/api/data", response_buffer, sizeof(response_buffer), &response);
    if (ret == IDCU_ERR_OK) {
        printf("Status: %d\n", response->status_code);
        printf("Body: %.*s\n\n", (int)response->body_length, response->body);
    } else {
        printf("Request failed: %d\n\n", ret);
    }
    
    idcu_http_client_destroy(&client);
    
    printf("Client example completed!\n");
    return 0;
}

int main(int argc, char* argv[]) {
    printf("====================================\n");
    printf("HTTP Network Example\n");
    printf("====================================\n\n");
    
    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        return run_http_server();
    } else if (argc > 1 && strcmp(argv[1], "client") == 0) {
        return run_http_client();
    } else {
        printf("Usage:\n");
        printf("  %s server  - Run HTTP server\n");
        printf("  %s client  - Run HTTP client (requires server to be running first)\n\n");
        printf("\nKey features demonstrated:\n");
        printf("  - HTTP server with multiple routes\n");
        printf("  - JSON request/response handling\n");
        printf("  - HTTP client GET/POST requests\n");
        printf("  - Custom headers and content types\n\n");
        return 0;
    }
}
