#include "idcu/http_server/http_server.h"
#include "idcu/log/log.h"
#include <stdio.h>

int hello_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    idcu_http_response_set_body(response, "Hello, World!", 13);
    return IDCU_ERR_OK;
}

int echo_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)user_data;
    if (request->body_length > 0) {
        idcu_http_response_set_body(response, request->body, request->body_length);
    } else {
        idcu_http_response_set_body(response, "No body received", 16);
    }
    return IDCU_ERR_OK;
}

int json_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    idcu_http_response_set_json_body(response, "{\"message\":\"Success\",\"status\":200}");
    return IDCU_ERR_OK;
}

int main(void) {
    printf("Starting simple HTTP server on 0.0.0.0:8080...\n");
    printf("Available routes:\n");
    printf("  GET  /          - Hello World\n");
    printf("  POST /echo      - Echo request body\n");
    printf("  GET  /api/json  - JSON response\n");
    printf("\nPress Ctrl+C to stop\n\n");

    idcu_HttpServer server;
    int ret = idcu_http_server_init(&server, "0.0.0.0", 8080);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize server: %d\n", ret);
        return 1;
    }

    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/", hello_handler, NULL);
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_POST, "/echo", echo_handler, NULL);
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/api/json", json_handler, NULL);

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
