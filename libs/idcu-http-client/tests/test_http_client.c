#include <idcu/testframework/testframework.h>
#include <idcu/http_client/http_client.h>
#include <idcu/http_client/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IDCU_TEST_CASE(http_client, init_destroy) {
    idcu_HttpClient client;
    int ret = idcu_http_client_init(&client);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_client_destroy(&client);
}

IDCU_TEST_CASE(http_client, request_init_destroy) {
    idcu_HttpClientRequest request;
    int ret = idcu_http_client_request_init(&request);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_client_request_destroy(&request);
}

IDCU_TEST_CASE(http_client, response_init_destroy) {
    idcu_HttpClientResponse response;
    int ret = idcu_http_client_response_init(&response);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_client_response_destroy(&response);
}

IDCU_TEST_CASE(http_client, request_set_method) {
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    
    int ret = idcu_http_client_request_set_method(&request, IDCU_HTTP_CLIENT_METHOD_GET);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_client_request_destroy(&request);
}

IDCU_TEST_CASE(http_client, request_set_url) {
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    
    int ret = idcu_http_client_request_set_url(&request, "http://example.com");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_client_request_destroy(&request);
}

IDCU_TEST_CASE(http_client, request_set_header) {
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    
    int ret = idcu_http_client_request_set_header(&request, "Content-Type", "application/json");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_client_request_destroy(&request);
}

IDCU_TEST_CASE(http_client, request_set_body) {
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    
    const char* body = "{\"key\": \"value\"}";
    int ret = idcu_http_client_request_set_body(&request, body, strlen(body));
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_client_request_destroy(&request);
}

IDCU_TEST_CASE(http_client, request_set_json) {
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    
    int ret = idcu_http_client_request_set_json(&request, "{\"key\": \"value\"}");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_http_client_request_destroy(&request);
}

IDCU_TEST_CASE(http_client, method_to_string) {
    const char* str = idcu_http_client_method_to_string(IDCU_HTTP_CLIENT_METHOD_GET);
    IDCU_TEST_ASSERT_STRING_EQUAL("GET", str);
    
    str = idcu_http_client_method_to_string(IDCU_HTTP_CLIENT_METHOD_POST);
    IDCU_TEST_ASSERT_STRING_EQUAL("POST", str);
}

IDCU_TEST_CASE(http_client, url_parse) {
    char host[256];
    uint16_t port;
    char path[1024];
    char query[1024];
    
    int ret = idcu_http_url_parse("http://example.com:8080/path?query=value", 
                                    host, sizeof(host), &port, 
                                    path, sizeof(path), query, sizeof(query));
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    IDCU_TEST_ASSERT_STRING_EQUAL("example.com", host);
    IDCU_TEST_ASSERT_EQUAL(8080, port);
}

int main(void) {
    return idcu_test_run_all();
}
