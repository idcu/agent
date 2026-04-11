#include <idcu/http_server/http_server.h>
#include <idcu/network/network.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static const char* method_strings[] = {
    "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"
};

static const char* status_reasons[] = {
    [200] = "OK",
    [201] = "Created",
    [202] = "Accepted",
    [204] = "No Content",
    [400] = "Bad Request",
    [401] = "Unauthorized",
    [403] = "Forbidden",
    [404] = "Not Found",
    [405] = "Method Not Allowed",
    [500] = "Internal Server Error",
    [501] = "Not Implemented",
    [503] = "Service Unavailable"
};

const char* idcu_http_method_to_string(idcu_HttpMethod method) {
    if (method >= 0 && method < (int)(sizeof(method_strings) / sizeof(method_strings[0]))) {
        return method_strings[method];
    }
    return "UNKNOWN";
}

const char* idcu_http_status_to_string(idcu_HttpStatus status) {
    if (status >= 100 && status < 600 && status_reasons[status]) {
        return status_reasons[status];
    }
    return "Unknown Status";
}

int idcu_http_request_init(idcu_HttpRequest* request) {
    if (!request) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(request, 0, sizeof(idcu_HttpRequest));
    request->method = IDCU_HTTP_METHOD_GET;
    strncpy(request->version, "HTTP/1.1", sizeof(request->version) - 1);
    
    return IDCU_ERR_OK;
}

void idcu_http_request_destroy(idcu_HttpRequest* request) {
    if (!request) {
        return;
    }
    
    if (request->body) {
        free(request->body);
        request->body = NULL;
    }
    
    memset(request, 0, sizeof(idcu_HttpRequest));
}

int idcu_http_request_get_header(const idcu_HttpRequest* request, const char* name, const char** value) {
    if (!request || !name || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    for (size_t i = 0; i < request->headers.count; i++) {
        if (strcasecmp(request->headers.headers[i].key, name) == 0) {
            *value = request->headers.headers[i].value;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

int idcu_http_request_get_query_param(const idcu_HttpRequest* request, const char* name, const char** value) {
    if (!request || !name || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    for (size_t i = 0; i < request->query_params.count; i++) {
        if (strcmp(request->query_params.params[i].key, name) == 0) {
            *value = request->query_params.params[i].value;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

int idcu_http_response_init(idcu_HttpResponse* response) {
    if (!response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(response, 0, sizeof(idcu_HttpResponse));
    response->status = IDCU_HTTP_STATUS_OK;
    strncpy(response->version, "HTTP/1.1", sizeof(response->version) - 1);
    strncpy(response->reason, "OK", sizeof(response->reason) - 1);
    
    return IDCU_ERR_OK;
}

void idcu_http_response_destroy(idcu_HttpResponse* response) {
    if (!response) {
        return;
    }
    
    if (response->body) {
        free(response->body);
        response->body = NULL;
    }
    
    memset(response, 0, sizeof(idcu_HttpResponse));
}

void idcu_http_response_set_status(idcu_HttpResponse* response, idcu_HttpStatus status) {
    if (!response) {
        return;
    }
    
    response->status = status;
    const char* reason = idcu_http_status_to_string(status);
    strncpy(response->reason, reason, sizeof(response->reason) - 1);
}

int idcu_http_response_set_header(idcu_HttpResponse* response, const char* name, const char* value) {
    if (!response || !name || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (response->headers.count >= IDCU_HTTP_HEADERS_MAX) {
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    strncpy(response->headers.headers[response->headers.count].key, name, IDCU_HTTP_HEADER_KEY_MAX - 1);
    strncpy(response->headers.headers[response->headers.count].value, value, IDCU_HTTP_HEADER_VALUE_MAX - 1);
    response->headers.count++;
    
    return IDCU_ERR_OK;
}

int idcu_http_response_set_body(idcu_HttpResponse* response, const void* data, size_t length) {
    if (!response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (response->body) {
        free(response->body);
    }
    
    if (data && length > 0) {
        response->body = (char*)malloc(length);
        if (!response->body) {
            response->body_length = 0;
            response->body_capacity = 0;
            return IDCU_ERR_MEMORY;
        }
        memcpy(response->body, data, length);
        response->body_length = length;
        response->body_capacity = length;
    } else {
        response->body = NULL;
        response->body_length = 0;
        response->body_capacity = 0;
    }
    
    return IDCU_ERR_OK;
}

int idcu_http_response_append_body(idcu_HttpResponse* response, const void* data, size_t length) {
    if (!response || !data || length == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t new_length = response->body_length + length;
    if (new_length > response->body_capacity) {
        size_t new_capacity = response->body_capacity ? response->body_capacity * 2 : 1024;
        if (new_capacity < new_length) {
            new_capacity = new_length;
        }
        char* new_body = (char*)realloc(response->body, new_capacity);
        if (!new_body) {
            return IDCU_ERR_MEMORY;
        }
        response->body = new_body;
        response->body_capacity = new_capacity;
    }
    
    memcpy(response->body + response->body_length, data, length);
    response->body_length = new_length;
    
    return IDCU_ERR_OK;
}

int idcu_http_response_set_string(idcu_HttpResponse* response, const char* str) {
    if (!response || !str) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    int ret = idcu_http_response_set_body(response, str, strlen(str));
    if (ret == IDCU_ERR_OK) {
        idcu_http_response_set_header(response, "Content-Type", "text/plain; charset=utf-8");
    }
    return ret;
}

int idcu_http_response_set_json(idcu_HttpResponse* response, const char* json) {
    if (!response || !json) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    int ret = idcu_http_response_set_body(response, json, strlen(json));
    if (ret == IDCU_ERR_OK) {
        idcu_http_response_set_header(response, "Content-Type", "application/json; charset=utf-8");
    }
    return ret;
}

void idcu_http_response_send_error(idcu_HttpResponse* response, idcu_HttpStatus status, const char* message) {
    if (!response) {
        return;
    }
    
    idcu_http_response_set_status(response, status);
    idcu_http_response_set_string(response, message ? message : idcu_http_status_to_string(status));
}

int idcu_http_server_init(idcu_HttpServer* server) {
    if (!server) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(server, 0, sizeof(idcu_HttpServer));
    
    int ret = idcu_vector_init(&server->routes, sizeof(idcu_HttpRoute), 16);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    ret = idcu_mutex_init(&server->lock);
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&server->routes);
        return ret;
    }
    
    ret = idcu_tcp_server_init(&server->server);
    if (ret != IDCU_ERR_OK) {
        idcu_mutex_destroy(&server->lock);
        idcu_vector_destroy(&server->routes);
        return ret;
    }
    
    server->initialized = 1;
    
    return IDCU_ERR_OK;
}

void idcu_http_server_destroy(idcu_HttpServer* server) {
    if (!server) {
        return;
    }
    
    idcu_http_server_stop(server);
    
    if (server->initialized) {
        idcu_tcp_server_destroy(&server->server);
        idcu_mutex_destroy(&server->lock);
        idcu_vector_destroy(&server->routes);
    }
    
    memset(server, 0, sizeof(idcu_HttpServer));
}

int idcu_http_server_set_static_dir(idcu_HttpServer* server, const char* dir) {
    if (!server || !server->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (dir) {
        strncpy(server->static_dir, dir, sizeof(server->static_dir) - 1);
        server->serve_static = 1;
    } else {
        server->static_dir[0] = '\0';
        server->serve_static = 0;
    }
    
    return IDCU_ERR_OK;
}

int idcu_http_server_add_route(idcu_HttpServer* server, idcu_HttpMethod method, const char* path, idcu_HttpHandler handler, void* user_data) {
    if (!server || !server->initialized || !path || !handler) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_HttpRoute route;
    memset(&route, 0, sizeof(route));
    route.method = method;
    strncpy(route.path, path, sizeof(route.path) - 1);
    route.handler = handler;
    route.user_data = user_data;
    
    idcu_mutex_lock(&server->lock);
    int ret = idcu_vector_push_back(&server->routes, &route);
    idcu_mutex_unlock(&server->lock);
    
    return ret;
}

int idcu_http_server_get(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data) {
    return idcu_http_server_add_route(server, IDCU_HTTP_METHOD_GET, path, handler, user_data);
}

int idcu_http_server_post(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data) {
    return idcu_http_server_add_route(server, IDCU_HTTP_METHOD_POST, path, handler, user_data);
}

int idcu_http_server_put(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data) {
    return idcu_http_server_add_route(server, IDCU_HTTP_METHOD_PUT, path, handler, user_data);
}

int idcu_http_server_delete(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data) {
    return idcu_http_server_add_route(server, IDCU_HTTP_METHOD_DELETE, path, handler, user_data);
}

static int parse_method(const char* str, idcu_HttpMethod* method) {
    if (!str || !method) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    for (size_t i = 0; i < sizeof(method_strings) / sizeof(method_strings[0]); i++) {
        if (strcmp(str, method_strings[i]) == 0) {
            *method = (idcu_HttpMethod)i;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_INVALID_ARG;
}

static int parse_query_params(idcu_HttpQueryParams* params, const char* query) {
    if (!params || !query) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char* copy = strdup(query);
    if (!copy) {
        return IDCU_ERR_MEMORY;
    }
    
    char* token = strtok(copy, "&");
    while (token && params->count < IDCU_HTTP_QUERY_PARAMS_MAX) {
        char* eq = strchr(token, '=');
        if (eq) {
            *eq = '\0';
            strncpy(params->params[params->count].key, token, IDCU_HTTP_QUERY_KEY_MAX - 1);
            strncpy(params->params[params->count].value, eq + 1, IDCU_HTTP_QUERY_VALUE_MAX - 1);
            params->count++;
        }
        token = strtok(NULL, "&");
    }
    
    free(copy);
    return IDCU_ERR_OK;
}

static int parse_request_line(idcu_HttpRequest* request, char* line) {
    if (!request || !line) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char* method_str = strtok(line, " ");
    char* path_str = strtok(NULL, " ");
    char* version_str = strtok(NULL, "\r\n");
    
    if (!method_str || !path_str) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    int ret = parse_method(method_str, &request->method);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    char* query = strchr(path_str, '?');
    if (query) {
        *query = '\0';
        parse_query_params(&request->query_params, query + 1);
    }
    
    strncpy(request->path, path_str, sizeof(request->path) - 1);
    
    if (version_str) {
        strncpy(request->version, version_str, sizeof(request->version) - 1);
    }
    
    return IDCU_ERR_OK;
}

static int parse_header(idcu_HttpHeaders* headers, char* line) {
    if (!headers || !line) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char* colon = strchr(line, ':');
    if (!colon) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    *colon = '\0';
    char* key = line;
    char* value = colon + 1;
    
    while (*value && isspace((unsigned char)*value)) {
        value++;
    }
    
    if (headers->count >= IDCU_HTTP_HEADERS_MAX) {
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    strncpy(headers->headers[headers->count].key, key, IDCU_HTTP_HEADER_KEY_MAX - 1);
    strncpy(headers->headers[headers->count].value, value, IDCU_HTTP_HEADER_VALUE_MAX - 1);
    headers->count++;
    
    return IDCU_ERR_OK;
}

static int parse_http_request(idcu_HttpRequest* request, const char* data, size_t length) {
    if (!request || !data || length == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_http_request_init(request);
    
    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        return IDCU_ERR_MEMORY;
    }
    memcpy(buffer, data, length);
    buffer[length] = '\0';
    
    char* line = buffer;
    char* next_line;
    
    next_line = strstr(line, "\r\n");
    if (next_line) {
        *next_line = '\0';
        int ret = parse_request_line(request, line);
        if (ret != IDCU_ERR_OK) {
            free(buffer);
            return ret;
        }
        line = next_line + 2;
    }
    
    while (line < buffer + length) {
        next_line = strstr(line, "\r\n");
        if (!next_line) {
            break;
        }
        
        *next_line = '\0';
        
        if (line[0] == '\0') {
            line = next_line + 2;
            break;
        }
        
        parse_header(&request->headers, line);
        line = next_line + 2;
    }
    
    size_t body_offset = line - buffer;
    if (body_offset < length) {
        size_t body_len = length - body_offset;
        request->body = (char*)malloc(body_len);
        if (request->body) {
            memcpy(request->body, line, body_len);
            request->body_length = body_len;
        }
    }
    
    free(buffer);
    return IDCU_ERR_OK;
}

static int find_route(idcu_HttpServer* server, idcu_HttpMethod method, const char* path, idcu_HttpRoute** route) {
    if (!server || !path || !route) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    *route = NULL;
    
    idcu_mutex_lock(&server->lock);
    
    IDCU_VECTOR_FOR_EACH(&server->routes, idcu_HttpRoute, r, i) {
        if (r->method == method && strcmp(r->path, path) == 0) {
            *route = r;
            break;
        }
    }
    
    idcu_mutex_unlock(&server->lock);
    
    return (*route != NULL) ? IDCU_ERR_OK : IDCU_ERR_NOT_FOUND;
}

static int format_http_response(const idcu_HttpResponse* response, char* buffer, size_t buffer_size, size_t* out_length) {
    if (!response || !buffer || !out_length) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    int written = snprintf(buffer, buffer_size, "%s %d %s\r\n", 
                           response->version, response->status, response->reason);
    if (written < 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t total = written;
    
    for (size_t i = 0; i < response->headers.count; i++) {
        written = snprintf(buffer + total, buffer_size - total, "%s: %s\r\n",
                          response->headers.headers[i].key, response->headers.headers[i].value);
        if (written < 0) {
            return IDCU_ERR_INVALID_ARG;
        }
        total += written;
    }
    
    written = snprintf(buffer + total, buffer_size - total, "Content-Length: %zu\r\n\r\n", response->body_length);
    if (written < 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    total += written;
    
    if (response->body && response->body_length > 0) {
        if (total + response->body_length > buffer_size) {
            return IDCU_ERR_BUFFER_TOO_SMALL;
        }
        memcpy(buffer + total, response->body, response->body_length);
        total += response->body_length;
    }
    
    *out_length = total;
    return IDCU_ERR_OK;
}

static void handle_connection(idcu_HttpServer* server, idcu_TcpSocket* client) {
    if (!server || !client) {
        return;
    }
    
    char buffer[8192];
    size_t received = 0;
    
    int ret = idcu_tcp_socket_recv(client, buffer, sizeof(buffer) - 1, &received);
    if (ret != IDCU_ERR_OK || received == 0) {
        return;
    }
    
    idcu_HttpRequest request;
    idcu_HttpResponse response;
    
    idcu_http_response_init(&response);
    
    ret = parse_http_request(&request, buffer, received);
    if (ret != IDCU_ERR_OK) {
        idcu_http_response_send_error(&response, IDCU_HTTP_STATUS_BAD_REQUEST, "Invalid request");
    } else {
        idcu_HttpRoute* route = NULL;
        ret = find_route(server, request.method, request.path, &route);
        
        if (ret == IDCU_ERR_OK && route) {
            route->handler(&request, &response, route->user_data);
        } else {
            idcu_http_response_send_error(&response, IDCU_HTTP_STATUS_NOT_FOUND, "Not Found");
        }
    }
    
    char response_buffer[65536];
    size_t response_len = 0;
    ret = format_http_response(&response, response_buffer, sizeof(response_buffer), &response_len);
    
    if (ret == IDCU_ERR_OK) {
        size_t sent = 0;
        idcu_tcp_socket_send_all(client, response_buffer, response_len);
    }
    
    idcu_http_request_destroy(&request);
    idcu_http_response_destroy(&response);
}

int idcu_http_server_listen(idcu_HttpServer* server, const char* host, uint16_t port, int backlog) {
    if (!server || !server->initialized || !host) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_NetAddress addr;
    int ret = idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, host, port);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    ret = idcu_tcp_server_listen(&server->server, &addr, backlog);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    server->running = 1;
    
    while (server->running) {
        idcu_TcpSocket client;
        idcu_tcp_socket_init(&client);
        
        ret = idcu_tcp_server_accept_timeout(&server->server, &client, 1000);
        if (ret == IDCU_ERR_OK) {
            handle_connection(server, &client);
            idcu_tcp_socket_destroy(&client);
        }
    }
    
    return IDCU_ERR_OK;
}

void idcu_http_server_stop(idcu_HttpServer* server) {
    if (!server) {
        return;
    }
    
    server->running = 0;
}
