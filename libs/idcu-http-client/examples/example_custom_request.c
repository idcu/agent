#include "idcu/http_client/http_client.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_HttpClient client;
    int ret = idcu_http_client_init(&client, 30000);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to initialize HTTP client\n");
        return 1;
    }

    idcu_HttpClientRequest req;
    idcu_http_client_request_init(&req, IDCU_HTTPC_METHOD_POST, "http://example.com/api/data");
    idcu_http_client_request_add_header(&req, "Content-Type", "application/json");
    idcu_http_client_request_add_header(&req, "User-Agent", "idcu-http-client/1.0");

    const char *body = "{\"key\": \"value\"}";
    idcu_http_client_request_set_body(&req, body, strlen(body));

    idcu_HttpClientResponse resp;
    idcu_http_client_response_init(&resp);

    printf("Making custom POST request...\n");
    ret = idcu_http_client_execute(&client, &req, &resp);

    if (ret == IDCU_ERR_OK) {
        printf("Status: %d %s\n", resp.status_code, resp.status_text);
    } else {
        printf("Request failed with error code: %d\n", ret);
    }

    idcu_http_client_request_destroy(&req);
    idcu_http_client_response_destroy(&resp);
    idcu_http_client_destroy(&client);
    return 0;
}
