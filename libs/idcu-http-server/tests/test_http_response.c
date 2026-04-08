#include "idcu/http_server/http_server.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("Testing HTTP response functions...\n");

    idcu_HttpResponse response;
    idcu_http_response_init(&response);
    printf("✓ Response initialized\n");

    assert(response.status_code == 200);
    assert(strcmp(response.status_text, "OK") == 0);
    printf("✓ Default status is 200 OK\n");

    int ret = idcu_http_response_set_status(&response, 404);
    assert(ret == IDCU_ERR_OK);
    assert(response.status_code == 404);
    assert(strcmp(response.status_text, "Not Found") == 0);
    printf("✓ Status set to 404 Not Found\n");

    ret = idcu_http_response_add_header(&response, "Content-Type", "text/plain");
    assert(ret == IDCU_ERR_OK);
    assert(response.header_count == 1);
    assert(strcmp(response.headers[0].name, "Content-Type") == 0);
    assert(strcmp(response.headers[0].value, "text/plain") == 0);
    printf("✓ Header added successfully\n");

    ret = idcu_http_response_set_body(&response, "Hello", 5);
    assert(ret == IDCU_ERR_OK);
    assert(response.body_length == 5);
    assert(strcmp(response.body, "Hello") == 0);
    printf("✓ Body set successfully\n");

    printf("All tests passed!\n");
    return 0;
}
