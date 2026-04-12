#include <idcu/testframework/testframework.h>
#include <idcu/http_server/http_server.h>
#include <idcu/http_server/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IDCU_TEST_CASE(http_server, init_destroy) {
    idcu_HttpServer server;
    int ret = idcu_http_server_init(&server);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_server_destroy(&server);
}

IDCU_TEST_CASE(http_server, request_init_destroy) {
    idcu_HttpRequest request;
    int ret = idcu_http_request_init(&request);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_request_destroy(&request);
}

IDCU_TEST_CASE(http_server, response_init_destroy) {
    idcu_HttpResponse response;
    int ret = idcu_http_response_init(&response);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_response_destroy(&response);
}

IDCU_TEST_CASE(http_server, response_set_status) {
    idcu_HttpResponse response;
    idcu_http_response_init(&response);
    
    idcu_http_response_set_status(&response, IDCU_HTTP_STATUS_OK);
    
    idcu_http_response_destroy(&response);
}

IDCU_TEST_CASE(http_server, response_set_body) {
    idcu_HttpResponse response;
    idcu_http_response_init(&response);
    
    const char* body = "Hello, World!";
    int ret = idcu_http_response_set_body(&response, body, strlen(body));
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_response_destroy(&response);
}

IDCU_TEST_CASE(http_server, response_set_string) {
    idcu_HttpResponse response;
    idcu_http_response_init(&response);
    
    int ret = idcu_http_response_set_string(&response, "Hello, World!");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_response_destroy(&response);
}

IDCU_TEST_CASE(http_server, response_set_header) {
    idcu_HttpResponse response;
    idcu_http_response_init(&response);
    
    int ret = idcu_http_response_set_header(&response, "Content-Type", "text/plain");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_response_destroy(&response);
}

IDCU_TEST_CASE(http_server, method_to_string) {
    const char* str = idcu_http_method_to_string(IDCU_HTTP_METHOD_GET);
    IDCU_TEST_ASSERT_STRING_EQUAL("GET", str);
    
    str = idcu_http_method_to_string(IDCU_HTTP_METHOD_POST);
    IDCU_TEST_ASSERT_STRING_EQUAL("POST", str);
}

IDCU_TEST_CASE(http_server, status_to_string) {
    const char* str = idcu_http_status_to_string(IDCU_HTTP_STATUS_OK);
    IDCU_TEST_ASSERT(str != NULL);
    
    str = idcu_http_status_to_string(IDCU_HTTP_STATUS_NOT_FOUND);
    IDCU_TEST_ASSERT(str != NULL);
}

int main(void) {
    return idcu_test_run_all();
}
