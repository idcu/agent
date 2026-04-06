#include "idcu/http_client/http_client.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("Testing HTTP client request...\n");
    
    idcu_HttpClientRequest req;
    int ret = idcu_http_client_request_init(&req, IDCU_HTTPC_METHOD_GET, "http://example.com/");
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: request init returned %d\n", ret);
        return 1;
    }
    
    if (strcmp(req.url, "http://example.com/") != 0) {
        printf("Test failed: URL mismatch\n");
        idcu_http_client_request_destroy(&req);
        return 1;
    }
    
    idcu_http_client_request_destroy(&req);
    printf("Test passed!\n");
    return 0;
}
