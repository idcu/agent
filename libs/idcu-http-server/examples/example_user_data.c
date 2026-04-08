#include "idcu/http_server/http_server.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *app_name;
    int request_count;
} AppContext;

int counter_handler(idcu_HttpRequest *request, idcu_HttpResponse *response, void *user_data) {
    (void)request;

    AppContext *ctx = (AppContext *)user_data;
    if (!ctx) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_body(response, "Internal error", 14);
        return IDCU_ERR_OK;
    }

    ctx->request_count++;

    char message[256];
    snprintf(message, sizeof(message), "%s: Request #%d", ctx->app_name, ctx->request_count);

    idcu_http_response_set_body(response, message, strlen(message));
    return IDCU_ERR_OK;
}

int main(void) {
    printf("Starting user data example on 0.0.0.0:8082...\n");
    printf("Visit / to see the request counter increment\n");
    printf("\nPress Ctrl+C to stop\n\n");

    AppContext ctx = {.app_name = "IDCU HTTP Server", .request_count = 0};

    idcu_HttpServer server;
    int ret = idcu_http_server_init(&server, "0.0.0.0", 8082);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize server: %d\n", ret);
        return 1;
    }

    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/", counter_handler, &ctx);

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
