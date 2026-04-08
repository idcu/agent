#include "idcu/http_server/http_server.h"
#include <stdio.h>
#include <string.h>

int api_handler(idcu_HttpRequest *request, idcu_HttpResponse *response, void *user_data) {
    (void)user_data;

    char message[256];
    snprintf(message, sizeof(message), "API request to: %s (method: %s)", request->path,
             request->method == IDCU_HTTP_METHOD_GET      ? "GET"
             : request->method == IDCU_HTTP_METHOD_POST   ? "POST"
             : request->method == IDCU_HTTP_METHOD_PUT    ? "PUT"
             : request->method == IDCU_HTTP_METHOD_DELETE ? "DELETE"
                                                          : "UNKNOWN");

    idcu_http_response_set_body(response, message, strlen(message));
    return IDCU_ERR_OK;
}

int main(void) {
    printf("Starting wildcard routes example on 0.0.0.0:8081...\n");
    printf("Try accessing:\n");
    printf("  GET  /api/users\n");
    printf("  GET  /api/products\n");
    printf("  POST /api/orders\n");
    printf("\nPress Ctrl+C to stop\n\n");

    idcu_HttpServer server;
    int ret = idcu_http_server_init(&server, "0.0.0.0", 8081);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize server: %d\n", ret);
        return 1;
    }

    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/api/*", api_handler, NULL);
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_POST, "/api/*", api_handler, NULL);
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_PUT, "/api/*", api_handler, NULL);
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_DELETE, "/api/*", api_handler, NULL);

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
