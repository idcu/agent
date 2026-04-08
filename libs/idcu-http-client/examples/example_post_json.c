#include "idcu/http_client/http_client.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    if (argc != 3) {
        printf("Usage: %s <url> <json_data>\n", argv[0]);
        printf("Example: %s http://example.com/api '{\"key\": \"value\"}'\n", argv[0]);
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

    printf("Making POST request to %s...\n", argv[1]);
    ret = idcu_http_client_post(&client, argv[1], argv[2], strlen(argv[2]), &resp);

    if (ret == IDCU_ERR_OK) {
        printf("Status: %d %s\n", resp.status_code, resp.status_text);
        printf("\nResponse:\n%s\n", resp.body);
    } else {
        printf("Request failed with error code: %d\n", ret);
    }

    idcu_http_client_response_destroy(&resp);
    idcu_http_client_destroy(&client);
    return 0;
}
