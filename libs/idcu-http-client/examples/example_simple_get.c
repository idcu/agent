#include "idcu/http_client/http_client.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    if (argc != 2) {
        printf("Usage: %s <url>\n", argv[0]);
        printf("Example: %s http://example.com\n", argv[0]);
        return 1;
    }
    
    idcu_HttpClient client;
    int ret = idcu_http_client_init(&client, 30000);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to initialize HTTP client\n");
        return 1;
    }
    
    idcu_HttpClientResponse resp;
    idcu_http_client_response_init(&resp);
    
    printf("Making GET request to %s...\n", argv[1]);
    ret = idcu_http_client_get(&client, argv[1], &resp);
    
    if (ret == IDCU_ERR_OK) {
        printf("Status: %d %s\n", resp.status_code, resp.status_text);
        printf("\nResponse body:\n%s\n", resp.body);
    } else {
        printf("Request failed with error code: %d\n", ret);
    }
    
    idcu_http_client_response_destroy(&resp);
    idcu_http_client_destroy(&client);
    return 0;
}
