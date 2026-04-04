#include "network/http_server.h"
#include "utils/log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char* g_status_texts[] = {
    [200] = "OK",
    [201] = "Created",
    [400] = "Bad Request",
    [404] = "Not Found",
    [405] = "Method Not Allowed",
    [500] = "Internal Server Error"
};

idcu_HttpMethod idcu_http_method_from_string(const char* method_str) {
    if (strcmp(method_str, "GET") == 0) return IDCU_HTTP_METHOD_GET;
    if (strcmp(method_str, "POST") == 0) return IDCU_HTTP_METHOD_POST;
    if (strcmp(method_str, "PUT") == 0) return IDCU_HTTP_METHOD_PUT;
    if (strcmp(method_str, "DELETE") == 0) return IDCU_HTTP_METHOD_DELETE;
    return IDCU_HTTP_METHOD_UNKNOWN;
}

const char* idcu_http_status_text(int status_code) {
    if (status_code >= 0 && status_code < (int)(sizeof(g_status_texts) / sizeof(g_status_texts[0])) && g_status_texts[status_code]) {
        return g_status_texts[status_code];
    }
    return "Unknown Status";
}

void idcu_http_response_init(idcu_HttpResponse* response) {
    if (!response) return;
    memset(response, 0, sizeof(idcu_HttpResponse));
    response->status_code = 200;
    response->status_text = "OK";
}

int idcu_http_response_set_status(idcu_HttpResponse* response, int status_code) {
    if (!response) return IDCU_ERR_INVALID_PARAM;
    response->status_code = status_code;
    response->status_text = idcu_http_status_text(status_code);
    return IDCU_ERR_OK;
}

int idcu_http_response_add_header(idcu_HttpResponse* response, const char* name, const char* value) {
    if (!response || !name || !value) return IDCU_ERR_INVALID_PARAM;
    if (response->header_count >= IDCU_HTTP_MAX_HEADER) return IDCU_ERR_QUEUE_FULL;
    
    strncpy(response->headers[response->header_count].name, name, IDCU_HTTP_MAX_HEADER - 1);
    strncpy(response->headers[response->header_count].value, value, IDCU_HTTP_MAX_HEADER_VALUE - 1);
    response->header_count++;
    return IDCU_ERR_OK;
}

int idcu_http_response_set_body(idcu_HttpResponse* response, const char* body, size_t length) {
    if (!response || !body) return IDCU_ERR_INVALID_PARAM;
    size_t copy_len = (length < IDCU_HTTP_MAX_BODY) ? length : IDCU_HTTP_MAX_BODY - 1;
    memcpy(response->body, body, copy_len);
    response->body[copy_len] = '\0';
    response->body_length = copy_len;
    return IDCU_ERR_OK;
}

int idcu_http_response_set_json_body(idcu_HttpResponse* response, const char* json) {
    if (!response || !json) return IDCU_ERR_INVALID_PARAM;
    int ret = idcu_http_response_set_body(response, json, strlen(json));
    if (ret == IDCU_ERR_OK) {
        idcu_http_response_add_header(response, "Content-Type", "application/json");
    }
    return ret;
}

static int parse_http_request(const char* buffer, size_t len, idcu_HttpRequest* request) {
    if (!buffer || !request) return IDCU_ERR_INVALID_PARAM;
    memset(request, 0, sizeof(idcu_HttpRequest));
    
    char* buf_copy = (char*)malloc(len + 1);
    if (!buf_copy) return IDCU_ERR_NO_MEMORY;
    memcpy(buf_copy, buffer, len);
    buf_copy[len] = '\0';
    
    char* line = strtok(buf_copy, "\r\n");
    if (!line) {
        free(buf_copy);
        return IDCU_ERR_INVALID_PARAM;
    }
    
    char method_str[IDCU_HTTP_MAX_METHOD];
    if (sscanf(line, "%15s %255s %15s", method_str, request->path, request->version) != 3) {
        free(buf_copy);
        return IDCU_ERR_INVALID_PARAM;
    }
    request->method = idcu_http_method_from_string(method_str);
    
    line = strtok(NULL, "\r\n");
    while (line && *line != '\0' && request->header_count < IDCU_HTTP_MAX_HEADER) {
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            strncpy(request->headers[request->header_count].name, line, IDCU_HTTP_MAX_HEADER - 1);
            const char* value_start = colon + 1;
            while (*value_start == ' ') value_start++;
            strncpy(request->headers[request->header_count].value, value_start, IDCU_HTTP_MAX_HEADER_VALUE - 1);
            request->header_count++;
        }
        line = strtok(NULL, "\r\n");
    }
    
    if (line) {
        char* body_start = strstr(buf_copy, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            size_t body_len = len - (body_start - buf_copy);
            if (body_len > 0 && body_len < IDCU_HTTP_MAX_BODY) {
                memcpy(request->body, body_start, body_len);
                request->body[body_len] = '\0';
                request->body_length = body_len;
            }
        }
    }
    
    free(buf_copy);
    return IDCU_ERR_OK;
}

static int send_http_response(idcu_NetworkSocket* sock, idcu_HttpResponse* response) {
    if (!sock || !response) return IDCU_ERR_INVALID_PARAM;
    
    char header_buf[4096];
    int offset = snprintf(header_buf, sizeof(header_buf),
                          "HTTP/1.1 %d %s\r\n",
                          response->status_code, response->status_text);
    
    for (int i = 0; i < response->header_count; i++) {
        offset += snprintf(header_buf + offset, sizeof(header_buf) - offset,
                          "%s: %s\r\n",
                          response->headers[i].name, response->headers[i].value);
    }
    
    if (response->body_length > 0) {
        offset += snprintf(header_buf + offset, sizeof(header_buf) - offset,
                          "Content-Length: %zu\r\n", response->body_length);
    }
    
    offset += snprintf(header_buf + offset, sizeof(header_buf) - offset, "\r\n");
    
    size_t sent;
    int ret = idcu_network_socket_send(sock, header_buf, offset, &sent);
    if (ret != IDCU_ERR_OK) return ret;
    
    if (response->body_length > 0) {
        ret = idcu_network_socket_send(sock, response->body, response->body_length, &sent);
    }
    
    return ret;
}

int idcu_http_server_init(idcu_HttpServer* server, idcu_MicroKernel* kernel, const char* address, uint16_t port) {
    if (!server || !kernel) return IDCU_ERR_INVALID_PARAM;
    
    memset(server, 0, sizeof(idcu_HttpServer));
    server->kernel = kernel;
    server->running = 0;
    
    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) return ret;
    
    ret = idcu_network_server_create(&server->server, IDCU_NET_PROTO_TCP, address, port);
    if (ret != IDCU_ERR_OK) return ret;
    
    IDCU_LOG_INFO("HTTP server initialized on %s:%d", address, port);
    return IDCU_ERR_OK;
}

void idcu_http_server_destroy(idcu_HttpServer* server) {
    if (!server) return;
    idcu_http_server_stop(server);
    idcu_network_server_destroy(&server->server);
    idcu_network_cleanup();
    memset(server, 0, sizeof(idcu_HttpServer));
}

int idcu_http_server_register_route(idcu_HttpServer* server, idcu_HttpMethod method,
                                    const char* path, idcu_HttpHandler handler, void* user_data) {
    if (!server || !path || !handler) return IDCU_ERR_INVALID_PARAM;
    if (server->route_count >= IDCU_HTTP_MAX_ROUTES) return IDCU_ERR_QUEUE_FULL;
    
    server->routes[server->route_count].method = method;
    strncpy(server->routes[server->route_count].path, path, IDCU_HTTP_MAX_PATH - 1);
    server->routes[server->route_count].handler = handler;
    server->routes[server->route_count].user_data = user_data;
    server->route_count++;
    
    IDCU_LOG_DEBUG("Registered route: %s %s", method == IDCU_HTTP_METHOD_GET ? "GET" :
                   method == IDCU_HTTP_METHOD_POST ? "POST" :
                   method == IDCU_HTTP_METHOD_PUT ? "PUT" :
                   method == IDCU_HTTP_METHOD_DELETE ? "DELETE" : "UNKNOWN", path);
    return IDCU_ERR_OK;
}

int idcu_http_server_start(idcu_HttpServer* server) {
    if (!server) return IDCU_ERR_INVALID_PARAM;
    if (server->running) return IDCU_ERR_OK;
    
    int ret = idcu_network_server_listen(&server->server);
    if (ret != IDCU_ERR_OK) return ret;
    
    server->running = 1;
    IDCU_LOG_INFO("HTTP server started");
    return IDCU_ERR_OK;
}

int idcu_http_server_stop(idcu_HttpServer* server) {
    if (!server) return IDCU_ERR_INVALID_PARAM;
    if (!server->running) return IDCU_ERR_OK;
    
    server->running = 0;
    idcu_network_server_close(&server->server);
    IDCU_LOG_INFO("HTTP server stopped");
    return IDCU_ERR_OK;
}

int idcu_http_server_poll(idcu_HttpServer* server, int timeout_ms) {
    if (!server) return IDCU_ERR_INVALID_PARAM;
    if (!server->running) return IDCU_ERR_NOT_INITIALIZED;
    
    idcu_NetworkSocket client_sock;
    int ret = idcu_network_server_accept(&server->server, &client_sock);
    
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
    char buffer[8192];
    size_t received;
    ret = idcu_network_socket_recv(&client_sock, buffer, sizeof(buffer) - 1, &received);
    
    if (ret != IDCU_ERR_OK || received == 0) {
        idcu_network_socket_close(&client_sock);
        return ret;
    }
    
    buffer[received] = '\0';
    IDCU_LOG_DEBUG("Received HTTP request: %.*s", (int)received, buffer);
    
    idcu_HttpRequest request;
    ret = parse_http_request(buffer, received, &request);
    
    idcu_HttpResponse response;
    idcu_http_response_init(&response);
    
    if (ret != IDCU_ERR_OK) {
        idcu_http_response_set_status(&response, 400);
        idcu_http_response_set_json_body(&response, "{\"error\":\"Bad Request\"}");
    } else {
        int handled = 0;
        for (int i = 0; i < server->route_count; i++) {
            if (server->routes[i].method == request.method &&
                strcmp(server->routes[i].path, request.path) == 0) {
                ret = server->routes[i].handler(&request, &response, server->routes[i].user_data);
                handled = 1;
                break;
            }
        }
        
        if (!handled) {
            idcu_http_response_set_status(&response, 404);
            idcu_http_response_set_json_body(&response, "{\"error\":\"Not Found\"}");
        }
    }
    
    send_http_response(&client_sock, &response);
    idcu_network_socket_close(&client_sock);
    
    return IDCU_ERR_OK;
}
