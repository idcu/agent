#include <idcu/rest_api/rest_api.h>
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
#include <idcu/common/string_buf.h>
#include <idcu/json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct idcu_RestApi {
    idcu_HttpServer http_server;
    idcu_Vector endpoints;
    idcu_Mutex lock;
    idcu_ApiCorsConfig cors_config;
    idcu_ApiEndpointId next_id;
    int initialized;
};

typedef struct {
    idcu_RestApi* api;
    idcu_ApiEndpoint* endpoint;
} idcu_RestApiHandlerContext;

static void rest_api_handler_wrapper(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    idcu_RestApiHandlerContext* ctx = (idcu_RestApiHandlerContext*)user_data;
    if (!ctx || !ctx->api || !ctx->endpoint) {
        idcu_http_response_send_error(response, IDCU_HTTP_STATUS_INTERNAL_SERVER_ERROR, "Invalid handler context");
        return;
    }

    if (ctx->api->cors_config.enabled) {
        idcu_http_response_set_header(response, "Access-Control-Allow-Origin", ctx->api->cors_config.allowed_origins);
        idcu_http_response_set_header(response, "Access-Control-Allow-Methods", ctx->api->cors_config.allowed_methods);
        idcu_http_response_set_header(response, "Access-Control-Allow-Headers", ctx->api->cors_config.allowed_headers);
        if (ctx->api->cors_config.allow_credentials) {
            idcu_http_response_set_header(response, "Access-Control-Allow-Credentials", "true");
        }
        char max_age[64];
        snprintf(max_age, sizeof(max_age), "%llu", (unsigned long long)ctx->api->cors_config.max_age);
        idcu_http_response_set_header(response, "Access-Control-Max-Age", max_age);
    }

    if (request->method == IDCU_HTTP_METHOD_OPTIONS) {
        idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
        return;
    }

    ctx->endpoint->handler(request, response, ctx->endpoint->user_data);
}

static idcu_ApiEndpoint* find_endpoint_by_id(idcu_RestApi* api, idcu_ApiEndpointId id) {
    IDCU_VECTOR_FOR_EACH(&api->endpoints, idcu_ApiEndpoint, endpoint, i) {
        if (endpoint->id == id) {
            return endpoint;
        }
    }
    return NULL;
}

int idcu_rest_api_init(idcu_RestApi** api) {
    if (!api) {
        return IDCU_ERR_INVALID_ARG;
    }

    *api = (idcu_RestApi*)malloc(sizeof(idcu_RestApi));
    if (!*api) {
        return IDCU_ERR_MEMORY;
    }

    memset(*api, 0, sizeof(idcu_RestApi));

    int ret = idcu_http_server_init(&(*api)->http_server);
    if (ret != IDCU_ERR_OK) {
        free(*api);
        *api = NULL;
        return ret;
    }

    ret = idcu_vector_init(&(*api)->endpoints, sizeof(idcu_ApiEndpoint), 16);
    if (ret != IDCU_ERR_OK) {
        idcu_http_server_destroy(&(*api)->http_server);
        free(*api);
        *api = NULL;
        return ret;
    }

    ret = idcu_mutex_init(&(*api)->lock);
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&(*api)->endpoints);
        idcu_http_server_destroy(&(*api)->http_server);
        free(*api);
        *api = NULL;
        return ret;
    }

    (*api)->cors_config.enabled = false;
    strncpy((*api)->cors_config.allowed_origins, "*", sizeof((*api)->cors_config.allowed_origins) - 1);
    strncpy((*api)->cors_config.allowed_methods, "GET,POST,PUT,DELETE,OPTIONS", sizeof((*api)->cors_config.allowed_methods) - 1);
    strncpy((*api)->cors_config.allowed_headers, "*", sizeof((*api)->cors_config.allowed_headers) - 1);
    (*api)->cors_config.allow_credentials = false;
    (*api)->cors_config.max_age = 86400;

    (*api)->next_id = 1;
    (*api)->initialized = 1;

    return IDCU_ERR_OK;
}

void idcu_rest_api_destroy(idcu_RestApi* api) {
    if (!api || !api->initialized) {
        return;
    }

    idcu_mutex_lock(&api->lock);

    idcu_http_server_destroy(&api->http_server);
    idcu_vector_destroy(&api->endpoints);

    idcu_mutex_unlock(&api->lock);
    idcu_mutex_destroy(&api->lock);

    memset(api, 0, sizeof(idcu_RestApi));
    free(api);
}

int idcu_rest_api_listen(idcu_RestApi* api, const char* host, uint16_t port, int backlog) {
    if (!api || !api->initialized || !host) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_http_server_listen(&api->http_server, host, port, backlog);
}

void idcu_rest_api_stop(idcu_RestApi* api) {
    if (!api || !api->initialized) {
        return;
    }

    idcu_http_server_stop(&api->http_server);
}

idcu_ApiEndpointId idcu_rest_api_add_endpoint(idcu_RestApi* api,
                                               const char* path,
                                               idcu_HttpMethod method,
                                               idcu_ApiHandler handler,
                                               void* user_data) {
    if (!api || !api->initialized || !path || !handler) {
        return 0;
    }

    idcu_mutex_lock(&api->lock);

    idcu_ApiEndpoint endpoint;
    memset(&endpoint, 0, sizeof(endpoint));

    endpoint.id = api->next_id++;
    strncpy(endpoint.path, path, sizeof(endpoint.path) - 1);
    endpoint.method = method;
    endpoint.handler = handler;
    endpoint.user_data = user_data;
    endpoint.auth.type = IDCU_API_AUTH_NONE;
    endpoint.rate_limit.enabled = false;
    strncpy(endpoint.version, "v1", sizeof(endpoint.version) - 1);

    idcu_vector_push_back(&api->endpoints, &endpoint);

    idcu_ApiEndpoint* stored_endpoint = (idcu_ApiEndpoint*)idcu_vector_get(&api->endpoints, idcu_vector_size(&api->endpoints) - 1);

    idcu_RestApiHandlerContext* ctx = (idcu_RestApiHandlerContext*)malloc(sizeof(idcu_RestApiHandlerContext));
    if (ctx) {
        ctx->api = api;
        ctx->endpoint = stored_endpoint;
        idcu_http_server_add_route(&api->http_server, method, path, rest_api_handler_wrapper, ctx);
    }

    idcu_mutex_unlock(&api->lock);

    return endpoint.id;
}

int idcu_rest_api_set_auth(idcu_RestApi* api, idcu_ApiEndpointId id, const idcu_ApiAuthConfig* config) {
    if (!api || !api->initialized || !config) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&api->lock);

    idcu_ApiEndpoint* endpoint = find_endpoint_by_id(api, id);
    if (!endpoint) {
        idcu_mutex_unlock(&api->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    memcpy(&endpoint->auth, config, sizeof(idcu_ApiAuthConfig));

    idcu_mutex_unlock(&api->lock);

    return IDCU_ERR_OK;
}

int idcu_rest_api_set_rate_limit(idcu_RestApi* api, idcu_ApiEndpointId id, const idcu_ApiRateLimitConfig* config) {
    if (!api || !api->initialized || !config) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&api->lock);

    idcu_ApiEndpoint* endpoint = find_endpoint_by_id(api, id);
    if (!endpoint) {
        idcu_mutex_unlock(&api->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    memcpy(&endpoint->rate_limit, config, sizeof(idcu_ApiRateLimitConfig));

    idcu_mutex_unlock(&api->lock);

    return IDCU_ERR_OK;
}

int idcu_rest_api_set_cors(idcu_RestApi* api, const idcu_ApiCorsConfig* config) {
    if (!api || !api->initialized || !config) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&api->lock);
    memcpy(&api->cors_config, config, sizeof(idcu_ApiCorsConfig));
    idcu_mutex_unlock(&api->lock);

    return IDCU_ERR_OK;
}

int idcu_rest_api_set_description(idcu_RestApi* api, idcu_ApiEndpointId id, const char* description) {
    if (!api || !api->initialized || !description) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&api->lock);

    idcu_ApiEndpoint* endpoint = find_endpoint_by_id(api, id);
    if (!endpoint) {
        idcu_mutex_unlock(&api->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    strncpy(endpoint->description, description, sizeof(endpoint->description) - 1);

    idcu_mutex_unlock(&api->lock);

    return IDCU_ERR_OK;
}

int idcu_rest_api_set_version(idcu_RestApi* api, idcu_ApiEndpointId id, const char* version) {
    if (!api || !api->initialized || !version) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&api->lock);

    idcu_ApiEndpoint* endpoint = find_endpoint_by_id(api, id);
    if (!endpoint) {
        idcu_mutex_unlock(&api->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    strncpy(endpoint->version, version, sizeof(endpoint->version) - 1);

    idcu_mutex_unlock(&api->lock);

    return IDCU_ERR_OK;
}

int idcu_rest_api_get(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data) {
    return idcu_rest_api_add_endpoint(api, path, IDCU_HTTP_METHOD_GET, handler, user_data) ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
}

int idcu_rest_api_post(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data) {
    return idcu_rest_api_add_endpoint(api, path, IDCU_HTTP_METHOD_POST, handler, user_data) ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
}

int idcu_rest_api_put(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data) {
    return idcu_rest_api_add_endpoint(api, path, IDCU_HTTP_METHOD_PUT, handler, user_data) ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
}

int idcu_rest_api_delete(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data) {
    return idcu_rest_api_add_endpoint(api, path, IDCU_HTTP_METHOD_DELETE, handler, user_data) ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
}

int idcu_rest_api_generate_openapi(idcu_RestApi* api, char* buffer, size_t buffer_size) {
    if (!api || !api->initialized || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&api->lock);

    idcu_StringBuf sb;
    int ret = idcu_string_buf_init(&sb, 65536);
    if (ret != IDCU_ERR_OK) {
        idcu_mutex_unlock(&api->lock);
        return ret;
    }

    idcu_string_buf_append(&sb, "{\n");
    idcu_string_buf_append(&sb, "  \"openapi\": \"3.0.0\",\n");
    idcu_string_buf_append(&sb, "  \"info\": {\n");
    idcu_string_buf_append(&sb, "    \"title\": \"IDCU REST API\",\n");
    idcu_string_buf_append(&sb, "    \"version\": \"1.0.0\"\n");
    idcu_string_buf_append(&sb, "  },\n");
    idcu_string_buf_append(&sb, "  \"paths\": {\n");

    IDCU_VECTOR_FOR_EACH(&api->endpoints, idcu_ApiEndpoint, endpoint, i) {
        if (i > 0) {
            idcu_string_buf_append(&sb, ",\n");
        }
        idcu_string_buf_append(&sb, "    \"");
        idcu_string_buf_append(&sb, endpoint->path);
        idcu_string_buf_append(&sb, "\": {\n");
        idcu_string_buf_append(&sb, "      \"");
        idcu_string_buf_append(&sb, idcu_http_method_to_string(endpoint->method));
        idcu_string_buf_append(&sb, "\": {\n");
        idcu_string_buf_append(&sb, "        \"summary\": \"");
        idcu_string_buf_append(&sb, endpoint->description[0] ? endpoint->description : "No description");
        idcu_string_buf_append(&sb, "\",\n");
        idcu_string_buf_append(&sb, "        \"responses\": {\n");
        idcu_string_buf_append(&sb, "          \"200\": {\n");
        idcu_string_buf_append(&sb, "            \"description\": \"Success\"\n");
        idcu_string_buf_append(&sb, "          }\n");
        idcu_string_buf_append(&sb, "        }\n");
        idcu_string_buf_append(&sb, "      }\n");
        idcu_string_buf_append(&sb, "    }");
    }

    idcu_string_buf_append(&sb, "\n  }\n");
    idcu_string_buf_append(&sb, "}\n");

    const char* data = idcu_string_buf_data(&sb);
    size_t len = idcu_string_buf_size(&sb);
    if (len >= buffer_size) {
        len = buffer_size - 1;
    }
    memcpy(buffer, data, len);
    buffer[len] = '\0';

    idcu_string_buf_destroy(&sb);

    idcu_mutex_unlock(&api->lock);

    return IDCU_ERR_OK;
}

char* idcu_rest_api_generate_openapi_alloc(idcu_RestApi* api) {
    if (!api || !api->initialized) {
        return NULL;
    }

    idcu_mutex_lock(&api->lock);

    idcu_StringBuf sb;
    int ret = idcu_string_buf_init(&sb, 65536);
    if (ret != IDCU_ERR_OK) {
        idcu_mutex_unlock(&api->lock);
        return NULL;
    }

    idcu_string_buf_append(&sb, "{\n");
    idcu_string_buf_append(&sb, "  \"openapi\": \"3.0.0\",\n");
    idcu_string_buf_append(&sb, "  \"info\": {\n");
    idcu_string_buf_append(&sb, "    \"title\": \"IDCU REST API\",\n");
    idcu_string_buf_append(&sb, "    \"version\": \"1.0.0\"\n");
    idcu_string_buf_append(&sb, "  },\n");
    idcu_string_buf_append(&sb, "  \"paths\": {\n");

    IDCU_VECTOR_FOR_EACH(&api->endpoints, idcu_ApiEndpoint, endpoint, i) {
        if (i > 0) {
            idcu_string_buf_append(&sb, ",\n");
        }
        idcu_string_buf_append(&sb, "    \"");
        idcu_string_buf_append(&sb, endpoint->path);
        idcu_string_buf_append(&sb, "\": {\n");
        idcu_string_buf_append(&sb, "      \"");
        idcu_string_buf_append(&sb, idcu_http_method_to_string(endpoint->method));
        idcu_string_buf_append(&sb, "\": {\n");
        idcu_string_buf_append(&sb, "        \"summary\": \"");
        idcu_string_buf_append(&sb, endpoint->description[0] ? endpoint->description : "No description");
        idcu_string_buf_append(&sb, "\",\n");
        idcu_string_buf_append(&sb, "        \"responses\": {\n");
        idcu_string_buf_append(&sb, "          \"200\": {\n");
        idcu_string_buf_append(&sb, "            \"description\": \"Success\"\n");
        idcu_string_buf_append(&sb, "          }\n");
        idcu_string_buf_append(&sb, "        }\n");
        idcu_string_buf_append(&sb, "      }\n");
        idcu_string_buf_append(&sb, "    }");
    }

    idcu_string_buf_append(&sb, "\n  }\n");
    idcu_string_buf_append(&sb, "}\n");

    const char* data = idcu_string_buf_data(&sb);
    size_t len = idcu_string_buf_size(&sb);
    char* buffer = (char*)malloc(len + 1);
    if (buffer) {
        memcpy(buffer, data, len);
        buffer[len] = '\0';
    }

    idcu_string_buf_destroy(&sb);

    idcu_mutex_unlock(&api->lock);

    return buffer;
}

int idcu_rest_api_response_json(idcu_HttpResponse* response, const char* json) {
    if (!response || !json) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
    return idcu_http_response_set_json(response, json);
}

int idcu_rest_api_response_error(idcu_HttpResponse* response, idcu_HttpStatus status, const char* message) {
    if (!response) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_http_response_send_error(response, status, message);
    return IDCU_ERR_OK;
}

int idcu_rest_api_response_success(idcu_HttpResponse* response, const char* data) {
    if (!response) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
    if (data) {
        idcu_http_response_set_string(response, data);
    }

    return IDCU_ERR_OK;
}
