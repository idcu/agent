#include "idcu/http_client/http_client.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("Testing HTTP client init...\n");
    
    idcu_HttpClient client;
    int ret = idcu_http_client_init(&client, 5000);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: init returned %d\n", ret);
        return 1;
    }
    
    idcu_http_client_destroy(&client);
    printf("Test passed!\n");
    return 0;
}
