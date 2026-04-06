#include "idcu/http_server/http_server.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("Testing HTTP method functions...\n");

    idcu_HttpMethod method;

    method = idcu_http_method_from_string("GET");
    assert(method == IDCU_HTTP_METHOD_GET);
    printf("✓ GET method parsed correctly\n");

    method = idcu_http_method_from_string("POST");
    assert(method == IDCU_HTTP_METHOD_POST);
    printf("✓ POST method parsed correctly\n");

    method = idcu_http_method_from_string("PUT");
    assert(method == IDCU_HTTP_METHOD_PUT);
    printf("✓ PUT method parsed correctly\n");

    method = idcu_http_method_from_string("DELETE");
    assert(method == IDCU_HTTP_METHOD_DELETE);
    printf("✓ DELETE method parsed correctly\n");

    method = idcu_http_method_from_string("UNKNOWN");
    assert(method == IDCU_HTTP_METHOD_UNKNOWN);
    printf("✓ Unknown method handled correctly\n");

    const char* text;
    text = idcu_http_status_text(200);
    assert(strcmp(text, "OK") == 0);
    printf("✓ Status text for 200 is correct\n");

    text = idcu_http_status_text(404);
    assert(strcmp(text, "Not Found") == 0);
    printf("✓ Status text for 404 is correct\n");

    text = idcu_http_status_text(999);
    assert(strcmp(text, "Unknown Status") == 0);
    printf("✓ Unknown status handled correctly\n");

    printf("All tests passed!\n");
    return 0;
}
