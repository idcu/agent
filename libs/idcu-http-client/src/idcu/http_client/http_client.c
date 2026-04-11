#include <idcu/http_client/http_client.h>
#include <idcu/network/network.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static const char* method_strings[] = {
    "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"
};

const char* idcu_http_client_method_to_string(idcu_HttpClientMethod method) {
    if (method >= 0 && method < (int)(sizeof(method_strings) / sizeof(method_strings[0]))) {
        return method_strings[method];
    }
    return "UNKNOWN";
}

int idcu_http_url_parse(const char* url, char* host, size_t host_size, uint16_t* port, char* path, size_t path_size, char* query, size_t query_size) {
    if (!url || !host || !port || !path || !query) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    const char* p = url;
    
    if (strncmp(p, "http://", 7) == 0) {
        p += 7;
    } else if (strncmp(p, "https://", 8) == 0) {
        p += 8;
    }
    
    const char* host_start = p;
    const char* host_end = p;
    const char* path_start = NULL;
    const char* query_start = NULL;
    
    while (*host_end && *host_end != '/' && *host_end != '?' && *host_end != ':') {
        host_end++;
    }
    
    if (*host_end == ':') {
        size_t host_len = host_end - host_start;
        if (host_len >= host_size) host_len = host_size - 1;
        strncpy(host, host_start, host_len);
        host[host_len] = '\0';
        
        host_end++;
        *port = (uint16_t)atoi(host_end);
        
        while (*host_end && isdigit((unsigned char)*host_end)) {
            host_end++;
        }
    } else {
        size_t host_len = host_end - host_start;
        if (host_len >= host_size) host_len = host_size - 1;
        strncpy(host, host_start, host_len);
        host[host_len] = '\0';
        *port = 80;
    }
    
    if (*host_end == '/') {
        path_start = host_end;
        const char* path_end = path_start;
        while (*path_end && *path_end != '?') {
            path_end++;
        }
        
        size_t path_len = path_end - path_start;
        if (path_len >= path_size) path_len = path_size - 1;
        strncpy(path, path_start, path_len);
        path[path_len] = '\0';
        
        if (*path_end == '?') {
            query_start = path_end + 1;
            size_t query_len = strlen(query_start);
            if (query_len >= query_size) query_len = query_size - 1;
            strncpy(query, query_start, query_len);
            query[query_len] = '\0';
        } else {
            query[0] = '\0';
        }
    } else {
        strncpy(path, "/", path_size - 1);
        path[path_size - 1] = '\0';
        query[0] = '\0';
    }
    
    return IDCU_ERR_OK;
}

int idcu_http_client_init(idcu_HttpClient* client) {
    if (!client) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(client, 0, sizeof(idcu_HttpClient));
    int ret = idcu_tcp_socket_init(&client->socket);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    client->timeout_ms = 30000;
    
    return IDCU_ERR_OK;
}

void idcu_http_client_destroy(idcu_HttpClient* client) {
    if (!client) {
        return;
    }
    
    idcu_http_client_disconnect(client);
    idcu_tcp_socket_destroy(&client->socket);
    memset(client, 0, sizeof(idcu_HttpClient));
}

int idcu_http_client_connect(idcu_HttpClient* client, const char* host, uint16_t port) {
    return idcu_http_client_connect_timeout(client, host, port, client->timeout_ms);
}

int idcu_http_client_connect_timeout(idcu_HttpClient* client, const char* host, uint16_t port, int timeout_ms) {
    if (!client || !host) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (client->connected) {
        idcu_http_client_disconnect(client);
    }
    
    idcu_NetAddress addr;
    int ret = idcu_net_address_resolve(&addr, host, port);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    ret = idcu_tcp_socket_connect_timeout(&client->socket, &addr, timeout_ms);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    strncpy(client->host, host, sizeof(client->host) - 1);
    client->port = port;
    client->timeout_ms = timeout_ms;
    client->connected = 1;
    
    return IDCU_ERR_OK;
}

void idcu_http_client_disconnect(idcu_HttpClient* client) {
    if (!client) {
        return;
    }
    
    if (client->connected) {
        idcu_tcp_socket_disconnect(&client->socket);
        client->connected = 0;
    }
}

int idcu_http_client_set_timeout(idcu_HttpClient* client, int timeout_ms) {
    if (!client) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    client->timeout_ms = timeout_ms;
    if (client->connected) {
        idcu_tcp_socket_set_timeout(&client->socket, timeout_ms);
    }
    
    return IDCU_ERR_OK;
}

int idcu_http_client_request_init(idcu_HttpClientRequest* request) {
    if (!request) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(request, 0, sizeof(idcu_HttpClientRequest));
    request->method = IDCU_HTTP_CLIENT_METHOD_GET;
    request->timeout_ms = 30000;
    request->follow_redirects = 1;
    request->max_redirects = 5;
    
    return IDCU_ERR_OK;
}

void idcu_http_client_request_destroy(idcu_HttpClientRequest* request) {
    if (!request) {
        return;
    }
    
    if (request->body) {
        free(request->body);
        request->body = NULL;
    }
    
    memset(request, 0, sizeof(idcu_HttpClientRequest));
}

int idcu_http_client_request_set_method(idcu_HttpClientRequest* request, idcu_HttpClientMethod method) {
    if (!request) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    request->method = method;
    return IDCU_ERR_OK;
}

int idcu_http_client_request_set_url(idcu_HttpClientRequest* request, const char* url) {
    if (!request || !url) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    strncpy(request->url, url, sizeof(request->url) - 1);
    return IDCU_ERR_OK;
}

int idcu_http_client_request_set_header(idcu_HttpClientRequest* request, const char* name, const char* value) {
    if (!request || !name || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (request->headers.count >= IDCU_HTTP_CLIENT_HEADERS_MAX) {
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    strncpy(request->headers.headers[request->headers.count].key, name, IDCU_HTTP_CLIENT_HEADER_KEY_MAX - 1);
    strncpy(request->headers.headers[request->headers.count].value, value, IDCU_HTTP_CLIENT_HEADER_VALUE_MAX - 1);
    request->headers.count++;
    
    return IDCU_ERR_OK;
}

int idcu_http_client_request_set_body(idcu_HttpClientRequest* request, const void* data, size_t length) {
    if (!request) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (request->body) {
        free(request->body);
    }
    
    if (data && length > 0) {
        request->body = (char*)malloc(length);
        if (!request->body) {
            request->body_length = 0;
            return IDCU_ERR_MEMORY;
        }
        memcpy(request->body, data, length);
        request->body_length = length;
    } else {
        request->body = NULL;
        request->body_length = 0;
    }
    
    return IDCU_ERR_OK;
}

int idcu_http_client_request_set_json(idcu_HttpClientRequest* request, const char* json) {
    if (!request || !json) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    int ret = idcu_http_client_request_set_body(request, json, strlen(json));
    if (ret == IDCU_ERR_OK) {
        idcu_http_client_request_set_header(request, "Content-Type", "application/json; charset=utf-8");
    }
    return ret;
}

int idcu_http_client_request_set_form(idcu_HttpClientRequest* request, const char* form_data) {
    if (!request || !form_data) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    int ret = idcu_http_client_request_set_body(request, form_data, strlen(form_data));
    if (ret == IDCU_ERR_OK) {
        idcu_http_client_request_set_header(request, "Content-Type", "application/x-www-form-urlencoded");
    }
    return ret;
}

int idcu_http_client_request_add_query_param(idcu_HttpClientRequest* request, const char* name, const char* value) {
    if (!request || !name || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t url_len = strlen(request->url);
    size_t name_len = strlen(name);
    size_t value_len = strlen(value);
    size_t needed = url_len + name_len + value_len + 3;
    
    if (needed >= sizeof(request->url)) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    
    if (strchr(request->url, '?')) {
        strncat(request->url, "&", sizeof(request->url) - url_len - 1);
    } else {
        strncat(request->url, "?", sizeof(request->url) - url_len - 1);
    }
    
    strncat(request->url, name, sizeof(request->url) - strlen(request->url) - 1);
    strncat(request->url, "=", sizeof(request->url) - strlen(request->url) - 1);
    strncat(request->url, value, sizeof(request->url) - strlen(request->url) - 1);
    
    return IDCU_ERR_OK;
}

int idcu_http_client_request_set_timeout(idcu_HttpClientRequest* request, int timeout_ms) {
    if (!request) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    request->timeout_ms = timeout_ms;
    return IDCU_ERR_OK;
}

int idcu_http_client_request_set_follow_redirects(idcu_HttpClientRequest* request, int follow, int max_redirects) {
    if (!request) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    request->follow_redirects = follow;
    request->max_redirects = max_redirects;
    return IDCU_ERR_OK;
}

int idcu_http_client_request_set_proxy(idcu_HttpClientRequest* request, const char* host, uint16_t port) {
    if (!request) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (host) {
        strncpy(request->proxy_host, host, sizeof(request->proxy_host) - 1);
        request->proxy_port = port;
        request->use_proxy = 1;
    } else {
        request->proxy_host[0] = '\0';
        request->proxy_port = 0;
        request->use_proxy = 0;
    }
    
    return IDCU_ERR_OK;
}

int idcu_http_client_response_init(idcu_HttpClientResponse* response) {
    if (!response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(response, 0, sizeof(idcu_HttpClientResponse));
    strncpy(response->version, "HTTP/1.1", sizeof(response->version) - 1);
    
    return IDCU_ERR_OK;
}

void idcu_http_client_response_destroy(idcu_HttpClientResponse* response) {
    if (!response) {
        return;
    }
    
    if (response->body) {
        free(response->body);
        response->body = NULL;
    }
    
    memset(response, 0, sizeof(idcu_HttpClientResponse));
}

int idcu_http_client_response_get_header(const idcu_HttpClientResponse* response, const char* name, const char** value) {
    if (!response || !name || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    for (size_t i = 0; i < response->headers.count; i++) {
        if (strcasecmp(response->headers.headers[i].key, name) == 0) {
            *value = response->headers.headers[i].value;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

int idcu_http_client_response_get_status_code(const idcu_HttpClientResponse* response) {
    if (!response) {
        return -1;
    }
    
    return response->status_code;
}

const char* idcu_http_client_response_get_body(const idcu_HttpClientResponse* response) {
    if (!response) {
        return NULL;
    }
    
    return response->body;
}

size_t idcu_http_client_response_get_body_length(const idcu_HttpClientResponse* response) {
    if (!response) {
        return 0;
    }
    
    return response->body_length;
}

static int build_http_request(const idcu_HttpClientRequest* request, char* buffer, size_t buffer_size, size_t* out_length) {
    if (!request || !buffer || !out_length) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char host[IDCU_HTTP_CLIENT_HOST_MAX];
    uint16_t port;
    char path[IDCU_HTTP_CLIENT_PATH_MAX];
    char query[IDCU_HTTP_CLIENT_QUERY_MAX];
    
    int ret = idcu_http_url_parse(request->url, host, sizeof(host), &port, path, sizeof(path), query, sizeof(query));
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    char full_path[IDCU_HTTP_CLIENT_PATH_MAX + IDCU_HTTP_CLIENT_QUERY_MAX + 2];
    if (strlen(query) > 0) {
        snprintf(full_path, sizeof(full_path), "%s?%s", path, query);
    } else {
        strncpy(full_path, path, sizeof(full_path) - 1);
    }
    
    int written = snprintf(buffer, buffer_size, "%s %s HTTP/1.1\r\n",
                          idcu_http_client_method_to_string(request->method),
                          full_path);
    if (written < 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t total = written;
    
    int has_host = 0;
    int has_content_length = 0;
    for (size_t i = 0; i < request->headers.count; i++) {
        if (strcasecmp(request->headers.headers[i].key, "Host") == 0) {
            has_host = 1;
        }
        if (strcasecmp(request->headers.headers[i].key, "Content-Length") == 0) {
            has_content_length = 1;
        }
    }
    
    if (!has_host) {
        written = snprintf(buffer + total, buffer_size - total, "Host: %s\r\n", host);
        if (written < 0) {
            return IDCU_ERR_INVALID_ARG;
        }
        total += written;
    }
    
    for (size_t i = 0; i < request->headers.count; i++) {
        written = snprintf(buffer + total, buffer_size - total, "%s: %s\r\n",
                          request->headers.headers[i].key,
                          request->headers.headers[i].value);
        if (written < 0) {
            return IDCU_ERR_INVALID_ARG;
        }
        total += written;
    }
    
    if (!has_content_length && request->body_length > 0) {
        written = snprintf(buffer + total, buffer_size - total, "Content-Length: %zu\r\n", request->body_length);
        if (written < 0) {
            return IDCU_ERR_INVALID_ARG;
        }
        total += written;
    }
    
    written = snprintf(buffer + total, buffer_size - total, "Connection: close\r\n\r\n");
    if (written < 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    total += written;
    
    if (request->body && request->body_length > 0) {
        if (total + request->body_length > buffer_size) {
            return IDCU_ERR_BUFFER_TOO_SMALL;
        }
        memcpy(buffer + total, request->body, request->body_length);
        total += request->body_length;
    }
    
    *out_length = total;
    return IDCU_ERR_OK;
}

static int parse_http_response(const char* data, size_t length, idcu_HttpClientResponse* response) {
    if (!data || length == 0 || !response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
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
        
        char* token = strtok(line, " ");
        if (token) {
            strncpy(response->version, token, sizeof(response->version) - 1);
            
            token = strtok(NULL, " ");
            if (token) {
                response->status_code = atoi(token);
                
                token = strtok(NULL, "\r\n");
                if (token) {
                    strncpy(response->reason, token, sizeof(response->reason) - 1);
                }
            }
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
        
        char* colon = strchr(line, ':');
        if (colon && response->headers.count < IDCU_HTTP_CLIENT_HEADERS_MAX) {
            *colon = '\0';
            char* key = line;
            char* value = colon + 1;
            
            while (*value && isspace((unsigned char)*value)) {
                value++;
            }
            
            strncpy(response->headers.headers[response->headers.count].key, key, IDCU_HTTP_CLIENT_HEADER_KEY_MAX - 1);
            strncpy(response->headers.headers[response->headers.count].value, value, IDCU_HTTP_CLIENT_HEADER_VALUE_MAX - 1);
            response->headers.count++;
        }
        
        line = next_line + 2;
    }
    
    size_t body_offset = line - buffer;
    if (body_offset < length) {
        size_t body_len = length - body_offset;
        response->body = (char*)malloc(body_len + 1);
        if (response->body) {
            memcpy(response->body, line, body_len);
            response->body[body_len] = '\0';
            response->body_length = body_len;
            response->body_capacity = body_len + 1;
        }
    }
    
    free(buffer);
    return IDCU_ERR_OK;
}

static int do_execute(idcu_HttpClient* client, const idcu_HttpClientRequest* request, idcu_HttpClientResponse* response, int redirect_count) {
    char host[IDCU_HTTP_CLIENT_HOST_MAX];
    uint16_t port;
    char path[IDCU_HTTP_CLIENT_PATH_MAX];
    char query[IDCU_HTTP_CLIENT_QUERY_MAX];
    
    int ret = idcu_http_url_parse(request->url, host, sizeof(host), &port, path, sizeof(path), query, sizeof(query));
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    strncpy(response->final_url, request->url, sizeof(response->final_url) - 1);
    response->redirect_count = redirect_count;
    
    int need_connect = !client->connected || strcmp(client->host, host) != 0 || client->port != port;
    if (need_connect) {
        ret = idcu_http_client_connect_timeout(client, host, port, request->timeout_ms);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }
    
    char request_buffer[65536];
    size_t request_len = 0;
    ret = build_http_request(request, request_buffer, sizeof(request_buffer), &request_len);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    ret = idcu_tcp_socket_send_all(&client->socket, request_buffer, request_len);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    char response_buffer[65536];
    size_t total_received = 0;
    size_t received = 0;
    
    while (1) {
        ret = idcu_tcp_socket_recv(&client->socket, response_buffer + total_received, 
                                  sizeof(response_buffer) - total_received - 1, &received);
        if (ret != IDCU_ERR_OK || received == 0) {
            break;
        }
        total_received += received;
    }
    
    if (total_received == 0) {
        return IDCU_ERR_NETWORK;
    }
    
    ret = parse_http_response(response_buffer, total_received, response);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    if (request->follow_redirects && 
        response->status_code >= 300 && 
        response->status_code < 400 && 
        redirect_count < request->max_redirects) {
        
        const char* location = NULL;
        ret = idcu_http_client_response_get_header(response, "Location", &location);
        if (ret == IDCU_ERR_OK && location) {
            idcu_HttpClientRequest new_request;
            idcu_http_client_request_init(&new_request);
            idcu_http_client_request_set_method(&new_request, request->method);
            idcu_http_client_request_set_url(&new_request, location);
            idcu_http_client_request_set_timeout(&new_request, request->timeout_ms);
            idcu_http_client_request_set_follow_redirects(&new_request, 1, request->max_redirects);
            
            idcu_HttpClientResponse new_response;
            idcu_http_client_response_init(&new_response);
            
            ret = do_execute(client, &new_request, &new_response, redirect_count + 1);
            
            idcu_http_client_request_destroy(&new_request);
            
            if (ret == IDCU_ERR_OK) {
                idcu_http_client_response_destroy(response);
                *response = new_response;
            } else {
                idcu_http_client_response_destroy(&new_response);
            }
        }
    }
    
    return IDCU_ERR_OK;
}

int idcu_http_client_execute(idcu_HttpClient* client, const idcu_HttpClientRequest* request, idcu_HttpClientResponse* response) {
    if (!client || !request || !response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_http_client_response_init(response);
    return do_execute(client, request, response, 0);
}

int idcu_http_client_get(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* response) {
    if (!client || !url || !response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    idcu_http_client_request_set_url(&request, url);
    idcu_http_client_request_set_method(&request, IDCU_HTTP_CLIENT_METHOD_GET);
    
    int ret = idcu_http_client_execute(client, &request, response);
    idcu_http_client_request_destroy(&request);
    
    return ret;
}

int idcu_http_client_post(idcu_HttpClient* client, const char* url, const void* data, size_t length, idcu_HttpClientResponse* response) {
    if (!client || !url || !response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    idcu_http_client_request_set_url(&request, url);
    idcu_http_client_request_set_method(&request, IDCU_HTTP_CLIENT_METHOD_POST);
    if (data && length > 0) {
        idcu_http_client_request_set_body(&request, data, length);
    }
    
    int ret = idcu_http_client_execute(client, &request, response);
    idcu_http_client_request_destroy(&request);
    
    return ret;
}

int idcu_http_client_post_json(idcu_HttpClient* client, const char* url, const char* json, idcu_HttpClientResponse* response) {
    if (!client || !url || !json || !response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    idcu_http_client_request_set_url(&request, url);
    idcu_http_client_request_set_method(&request, IDCU_HTTP_CLIENT_METHOD_POST);
    idcu_http_client_request_set_json(&request, json);
    
    int ret = idcu_http_client_execute(client, &request, response);
    idcu_http_client_request_destroy(&request);
    
    return ret;
}

int idcu_http_client_put(idcu_HttpClient* client, const char* url, const void* data, size_t length, idcu_HttpClientResponse* response) {
    if (!client || !url || !response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    idcu_http_client_request_set_url(&request, url);
    idcu_http_client_request_set_method(&request, IDCU_HTTP_CLIENT_METHOD_PUT);
    if (data && length > 0) {
        idcu_http_client_request_set_body(&request, data, length);
    }
    
    int ret = idcu_http_client_execute(client, &request, response);
    idcu_http_client_request_destroy(&request);
    
    return ret;
}

int idcu_http_client_delete(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* response) {
    if (!client || !url || !response) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    idcu_http_client_request_set_url(&request, url);
    idcu_http_client_request_set_method(&request, IDCU_HTTP_CLIENT_METHOD_DELETE);
    
    int ret = idcu_http_client_execute(client, &request, response);
    idcu_http_client_request_destroy(&request);
    
    return ret;
}
