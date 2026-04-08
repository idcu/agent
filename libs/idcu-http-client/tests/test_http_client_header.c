#include "idcu/http_client/http_client.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("Testing HTTP client headers...\n");

    idcu_HttpClientRequest req;
    idcu_http_client_request_init(&req, IDCU_HTTPC_METHOD_GET, "http://example.com");

    int ret = idcu_http_client_request_add_header(&req, "Content-Type", "application/json");
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: add header returned %d\n", ret);
        idcu_http_client_request_destroy(&req);
        return 1;
    }

    if (req.header_count != 1) {
        printf("Test failed: header count should be 1, got %zu\n", req.header_count);
        idcu_http_client_request_destroy(&req);
        return 1;
    }

    idcu_http_client_request_destroy(&req);
    printf("Test passed!\n");
    return 0;
}
