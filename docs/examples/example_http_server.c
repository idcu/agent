/**
 * @file example_http_server.c
 * @brief HTTP Server example
 */

#include <idcu/http_server/http_server.h>
#include <idcu/log/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Handler for GET /hello
void hello_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    
    idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
    idcu_http_response_set_string(response, "Hello, World!");
}

// Handler for GET /api/user
void user_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    
    const char* json = 
        "{\n"
        "  \"id\": 1,\n"
        "  \"name\": \"John Doe\",\n"
        "  \"email\": \"john@example.com\"\n"
        "}";
    
    idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
    idcu_http_response_set_json(response, json);
}

// Handler for POST /api/data
void data_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)user_data;
    
    // Get query parameter
    const char* param = NULL;
    if (idcu_http_request_get_query_param(request, "id", &param) == IDCU_ERR_OK) {
        IDCU_LOG_INFO("Received request with id: %s", param);
    }
    
    // Echo back the body if present
    if (request->body && request->body_length > 0) {
        idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
        idcu_http_response_set_body(response, request->body, request->body_length);
        idcu_http_response_set_header(response, "Content-Type", "application/json");
    } else {
        idcu_http_response_send_error(response, IDCU_HTTP_STATUS_BAD_REQUEST, "No data received");
    }
}

int main(void) {
    printf("=== HTTP Server Example ===\n\n");

    // Initialize logging
    idcu_log_init(NULL, IDCU_LOG_INFO);

    // Create HTTP server
    idcu_HttpServer server;
    int ret = idcu_http_server_init(&server);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize HTTP server\n");
        return 1;
    }

    // Register routes
    printf("Registering routes...\n");
    
    idcu_http_server_get(&server, "/hello", hello_handler, NULL);
    printf("  GET /hello -> hello_handler\n");
    
    idcu_http_server_get(&server, "/api/user", user_handler, NULL);
    printf("  GET /api/user -> user_handler\n");
    
    idcu_http_server_post(&server, "/api/data", data_handler, NULL);
    printf("  POST /api/data -> data_handler\n");

    // Optional: Set static directory
    // idcu_http_server_set_static_dir(&server, "./public");

    printf("\nServer configuration complete\n");
    printf("\nTo run the server:\n");
    printf("  idcu_http_server_listen(&server, \"0.0.0.0\", 8080, 10);\n");
    printf("\nThis would start the server on port 8080\n");
    printf("Press Ctrl+C to stop the server\n");

    // Note: In a real application, you would call:
    // idcu_http_server_listen(&server, "0.0.0.0", 8080, 10);

    // Cleanup
    idcu_http_server_destroy(&server);
    idcu_log_shutdown();

    printf("\n=== Example completed ===\n");
    return 0;
}
