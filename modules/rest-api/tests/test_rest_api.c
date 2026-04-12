#include <idcu/testframework/testframework.h>
#include <idcu/rest_api/rest_api.h>
#include <string.h>

static int g_handler_called = 0;

static void test_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)response;
    (void)user_data;
    g_handler_called++;
}

IDCU_TEST_CASE(rest_api, init_destroy) {
    idcu_RestApi* api = NULL;
    int ret = idcu_rest_api_init(&api);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(api != NULL);

    idcu_rest_api_destroy(api);
}

IDCU_TEST_CASE(rest_api, add_endpoint) {
    idcu_RestApi* api = NULL;
    int ret = idcu_rest_api_init(&api);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    idcu_ApiEndpointId id = idcu_rest_api_add_endpoint(api, "/test", IDCU_HTTP_METHOD_GET, test_handler, NULL);
    IDCU_TEST_ASSERT(id != 0);

    idcu_rest_api_destroy(api);
}

IDCU_TEST_CASE(rest_api, convenience_methods) {
    idcu_RestApi* api = NULL;
    int ret = idcu_rest_api_init(&api);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_rest_api_get(api, "/get", test_handler, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_rest_api_post(api, "/post", test_handler, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_rest_api_put(api, "/put", test_handler, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_rest_api_delete(api, "/delete", test_handler, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    idcu_rest_api_destroy(api);
}

IDCU_TEST_CASE(rest_api, set_cors) {
    idcu_RestApi* api = NULL;
    int ret = idcu_rest_api_init(&api);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    idcu_ApiCorsConfig cors_config = {0};
    cors_config.enabled = true;
    strncpy(cors_config.allowed_origins, "https://example.com", sizeof(cors_config.allowed_origins) - 1);
    strncpy(cors_config.allowed_methods, "GET,POST", sizeof(cors_config.allowed_methods) - 1);
    strncpy(cors_config.allowed_headers, "Content-Type", sizeof(cors_config.allowed_headers) - 1);
    cors_config.allow_credentials = true;
    cors_config.max_age = 3600;

    ret = idcu_rest_api_set_cors(api, &cors_config);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    idcu_rest_api_destroy(api);
}

IDCU_TEST_CASE(rest_api, set_description_and_version) {
    idcu_RestApi* api = NULL;
    int ret = idcu_rest_api_init(&api);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    idcu_ApiEndpointId id = idcu_rest_api_add_endpoint(api, "/test", IDCU_HTTP_METHOD_GET, test_handler, NULL);
    IDCU_TEST_ASSERT(id != 0);

    ret = idcu_rest_api_set_description(api, id, "Test endpoint description");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_rest_api_set_version(api, id, "v2");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    idcu_rest_api_destroy(api);
}

IDCU_TEST_CASE(rest_api, generate_openapi) {
    idcu_RestApi* api = NULL;
    int ret = idcu_rest_api_init(&api);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);

    idcu_rest_api_get(api, "/api/v1/status", test_handler, NULL);
    idcu_rest_api_get(api, "/api/v1/modules", test_handler, NULL);

    char buffer[65536];
    ret = idcu_rest_api_generate_openapi(api, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK);
    IDCU_TEST_ASSERT(strlen(buffer) > 0);

    char* openapi = idcu_rest_api_generate_openapi_alloc(api);
    IDCU_TEST_ASSERT(openapi != NULL);
    IDCU_TEST_ASSERT(strlen(openapi) > 0);
    free(openapi);

    idcu_rest_api_destroy(api);
}

IDCU_TEST_CASE(rest_api, error_cases) {
    int ret = idcu_rest_api_init(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_rest_api_destroy(NULL);

    ret = idcu_rest_api_listen(NULL, "0.0.0.0", 8080, 10);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_rest_api_stop(NULL);

    idcu_ApiEndpointId id = idcu_rest_api_add_endpoint(NULL, "/test", IDCU_HTTP_METHOD_GET, test_handler, NULL);
    IDCU_TEST_ASSERT(id == 0);

    id = idcu_rest_api_add_endpoint(NULL, NULL, IDCU_HTTP_METHOD_GET, test_handler, NULL);
    IDCU_TEST_ASSERT(id == 0);

    id = idcu_rest_api_add_endpoint(NULL, "/test", IDCU_HTTP_METHOD_GET, NULL, NULL);
    IDCU_TEST_ASSERT(id == 0);

    idcu_ApiAuthConfig auth_config = {0};
    ret = idcu_rest_api_set_auth(NULL, 1, &auth_config);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_ApiRateLimitConfig rate_config = {0};
    ret = idcu_rest_api_set_rate_limit(NULL, 1, &rate_config);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_ApiCorsConfig cors_config = {0};
    ret = idcu_rest_api_set_cors(NULL, &cors_config);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_rest_api_set_description(NULL, 1, "test");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_rest_api_set_version(NULL, 1, "v1");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_rest_api_get(NULL, "/test", test_handler, NULL);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK);

    ret = idcu_rest_api_post(NULL, "/test", test_handler, NULL);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK);

    ret = idcu_rest_api_put(NULL, "/test", test_handler, NULL);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK);

    ret = idcu_rest_api_delete(NULL, "/test", test_handler, NULL);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK);

    char buffer[1024];
    ret = idcu_rest_api_generate_openapi(NULL, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    char* openapi = idcu_rest_api_generate_openapi_alloc(NULL);
    IDCU_TEST_ASSERT(openapi == NULL);
}

int main(void) {
    return idcu_test_run_all();
}
