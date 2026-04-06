#include "idcu/http_client/http_client.h"
#include "idcu/log/log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static int parse_url(const char* url, char* host, size_t host_len, uint16_t* port, char* path, size_t path_len) {
    if (!url || !host || !port || !path) {
        return IDCU_ERR_INVALID_PARAM;
    }

    const char* ptr = url;
    if (strncmp(url, "http://", 7) == 0) {
        ptr += 7;
        *port = 80;
    } else if (strncmp(url, "https://", 8) == 0) {
        ptr += 8;
        *port = 443;
        IDCU_LOG_WARN("HTTPS not yet supported, falling back to HTTP");
    } else {
        *port = 80;
    }

    const char* host_start = ptr;
    const char* path_start = strchr(ptr, '/');
    const char* port_start = strchr(ptr, ':');

    if (port_start && (!path_start || port_start < path_start)) {
        size_t host_part_len = port_start - host_start;
        if (host_part_len >= host_len) host_part_len = host_len - 1;
        strncpy(host, host_start, host_part_len);
        host[host_part_len] = '\0';
        *port = (uint16_t)atoi(port_start + 1);
    } else {
        size_t host_part_len = path_start ? (path_start - host_start) : strlen(host_start);
        if (host_part_len >= host_len) host_part_len = host_len - 1;
        strncpy(host, host_start, host_part_len);
        host[host_part_len] = '\0';
    }

    if (path_start) {
        strncpy(path, path_start, path_len - 1);
        path[path_len - 1] = '\0';
    } else {
        strncpy(path, "/", path_len - 1);
        path[path_len - 1] = '\0';
    }

    return IDCU_ERR_OK;
}

const char* idcu_http_client_method_to_string(idcu_HttpClientMethod method) {
    switch (method) {
        case IDCU_HTTPC_METHOD_GET: return "GET";
        case IDCU_HTTPC_METHOD_POST: return "POST";
        case IDCU_HTTPC_METHOD_PUT: return "PUT";
        case IDCU_HTTPC_METHOD_DELETE: return "DELETE";
        case IDCU_HTTPC_METHOD_PATCH: return "PATCH";
        case IDCU_HTTPC_METHOD_HEAD: return "HEAD";
        case IDCU_HTTPC_METHOD_OPTIONS: return "OPTIONS";
        default: return "GET";
    }
}

int idcu_http_client_init(idcu_HttpClient* client, int timeout_ms) {
    if (!client) return IDCU_ERR_INVALID_PARAM;

    memset(client, 0, sizeof(idcu_HttpClient));
    client->timeout_ms = timeout_ms > 0 ? timeout_ms : 30000;
    client->connected = 0;

    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    return IDCU_ERR_OK;
}

void idcu_http_client_destroy(idcu_HttpClient* client) {
    if (!client) return;

    if (client->connected) {
        idcu_network_socket_close(&client->socket);
    }

    idcu_network_cleanup();
}

int idcu_http_client_request_init(idcu_HttpClientRequest* req, idcu_HttpClientMethod method, const char* url) {
    if (!req || !url) return IDCU_ERR_INVALID_PARAM;

    memset(req, 0, sizeof(idcu_HttpClientRequest));
    req->method = method;
    strncpy(req->url, url, IDCU_HTTPC_URL_MAX - 1);
    req->url[IDCU_HTTPC_URL_MAX - 1] = '\0';
    req->header_count = 0;
    req->body = NULL;
    req->body_length = 0;

    return IDCU_ERR_OK;
}

void idcu_http_client_request_destroy(idcu_HttpClientRequest* req) {
    if (!req) return;
    if (req->body) {
        free(req->body);
        req->body = NULL;
    }
}

int idcu_http_client_request_add_header(idcu_HttpClientRequest* req, const char* key, const char* value) {
    if (!req || !key || !value) return IDCU_ERR_INVALID_PARAM;
    if (req->header_count >= IDCU_HTTPC_MAX_HEADERS) return IDCU_ERR_NO_MEMORY;

    strncpy(req->headers[req->header_count].key, key, IDCU_HTTPC_HEADER_KEY_MAX - 1);
    req->headers[req->header_count].key[IDCU_HTTPC_HEADER_KEY_MAX - 1] = '\0';
    strncpy(req->headers[req->header_count].value, value, IDCU_HTTPC_HEADER_VALUE_MAX - 1);
    req->headers[req->header_count].value[IDCU_HTTPC_HEADER_VALUE_MAX - 1] = '\0';
    req->header_count++;

    return IDCU_ERR_OK;
}

int idcu_http_client_request_set_body(idcu_HttpClientRequest* req, const char* body, size_t length) {
    if (!req) return IDCU_ERR_INVALID_PARAM;

    if (req->body) {
        free(req->body);
        req->body = NULL;
    }

    if (body && length > 0) {
        req->body = (char*)malloc(length);
        if (!req->body) return IDCU_ERR_NO_MEMORY;
        memcpy(req->body, body, length);
        req->body_length = length;
    }

    return IDCU_ERR_OK;
}

int idcu_http_client_response_init(idcu_HttpClientResponse* resp) {
    if (!resp) return IDCU_ERR_INVALID_PARAM;
    memset(resp, 0, sizeof(idcu_HttpClientResponse));
    return IDCU_ERR_OK;
}

void idcu_http_client_response_destroy(idcu_HttpClientResponse* resp) {
    if (!resp) return;
}

const char* idcu_http_client_response_get_header(idcu_HttpClientResponse* resp, const char* key) {
    if (!resp || !key) return NULL;

    for (size_t i = 0; i < resp->header_count; i++) {
        if (strcasecmp(resp->headers[i].key, key) == 0) {
            return resp->headers[i].value;
        }
    }
    return NULL;
}

static int build_request_string(idcu_HttpClientRequest* req, const char* host, const char* path, char* buffer, size_t buffer_len, size_t* out_len) {
    size_t offset = 0;

    offset += snprintf(buffer + offset, buffer_len - offset,
                      "%s %s HTTP/1.1\r\n",
                      idcu_http_client_method_to_string(req->method), path);

    offset += snprintf(buffer + offset, buffer_len - offset,
                      "Host: %s\r\n", host);

    int has_content_length = 0;
    int has_connection = 0;

    for (size_t i = 0; i < req->header_count; i++) {
        if (strcasecmp(req->headers[i].key, "Content-Length") == 0) {
            has_content_length = 1;
        }
        if (strcasecmp(req->headers[i].key, "Connection") == 0) {
            has_connection = 1;
        }
        offset += snprintf(buffer + offset, buffer_len - offset,
                          "%s: %s\r\n",
                          req->headers[i].key, req->headers[i].value);
    }

    if (!has_connection) {
        offset += snprintf(buffer + offset, buffer_len - offset, "Connection: close\r\n");
    }

    if (req->body && req->body_length > 0 && !has_content_length) {
        offset += snprintf(buffer + offset, buffer_len - offset,
                          "Content-Length: %zu\r\n", req->body_length);
    }

    offset += snprintf(buffer + offset, buffer_len - offset, "\r\n");

    if (req->body && req->body_length > 0) {
        if (offset + req->body_length >= buffer_len) {
            return IDCU_ERR_NO_MEMORY;
        }
        memcpy(buffer + offset, req->body, req->body_length);
        offset += req->body_length;
    }

    *out_len = offset;
    return IDCU_ERR_OK;
}

static int parse_response_line(char* line, idcu_HttpClientResponse* resp) {
    char* ptr = line;
    while (*ptr && !isspace((unsigned char)*ptr)) ptr++;
    if (!*ptr) return IDCU_ERR_INVALID_PARAM;
    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

    resp->status_code = atoi(ptr);
    while (*ptr && isdigit((unsigned char)*ptr)) ptr++;
    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

    strncpy(resp->status_text, ptr, sizeof(resp->status_text) - 1);
    char* newline = strchr(resp->status_text, '\r');
    if (newline) *newline = '\0';
    newline = strchr(resp->status_text, '\n');
    if (newline) *newline = '\0';

    return IDCU_ERR_OK;
}

static int parse_header_line(char* line, idcu_HttpClientResponse* resp) {
    if (resp->header_count >= IDCU_HTTPC_MAX_HEADERS) return IDCU_ERR_OK;

    char* colon = strchr(line, ':');
    if (!colon) return IDCU_ERR_OK;

    *colon = '\0';
    char* key = line;
    char* value = colon + 1;

    while (*value && isspace((unsigned char)*value)) value++;

    char* newline = strchr(value, '\r');
    if (newline) *newline = '\0';
    newline = strchr(value, '\n');
    if (newline) *newline = '\0';

    strncpy(resp->headers[resp->header_count].key, key, IDCU_HTTPC_HEADER_KEY_MAX - 1);
    resp->headers[resp->header_count].key[IDCU_HTTPC_HEADER_KEY_MAX - 1] = '\0';
    strncpy(resp->headers[resp->header_count].value, value, IDCU_HTTPC_HEADER_VALUE_MAX - 1);
    resp->headers[resp->header_count].value[IDCU_HTTPC_HEADER_VALUE_MAX - 1] = '\0';
    resp->header_count++;

    return IDCU_ERR_OK;
}

static int receive_response(idcu_HttpClient* client, idcu_HttpClientResponse* resp) {
    char buffer[IDCU_HTTPC_RESPONSE_BUF_SIZE];
    size_t total_received = 0;
    int headers_complete = 0;
    char* body_start = NULL;
    size_t content_length = 0;
    int has_content_length = 0;

    while (!headers_complete || (has_content_length && total_received < content_length)) {
        size_t received = 0;
        int ret = idcu_network_socket_recv(&client->socket, buffer + total_received,
                                          sizeof(buffer) - total_received - 1, &received);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }

        if (received == 0) {
            break;
        }

        total_received += received;
        buffer[total_received] = '\0';

        if (!headers_complete) {
            body_start = strstr(buffer, "\r\n\r\n");
            if (body_start) {
                headers_complete = 1;
                *body_start = '\0';
                body_start += 4;

                char* line = strtok(buffer, "\r\n");
                if (line) {
                    parse_response_line(line, resp);
                    line = strtok(NULL, "\r\n");
                }

                while (line) {
                    parse_header_line(line, resp);
                    line = strtok(NULL, "\r\n");
                }

                const char* cl_header = idcu_http_client_response_get_header(resp, "Content-Length");
                if (cl_header) {
                    content_length = atoi(cl_header);
                    has_content_length = 1;
                }

                size_t body_in_buffer = total_received - (body_start - buffer);
                if (body_in_buffer > 0) {
                    size_t copy_len = body_in_buffer;
                    if (copy_len > IDCU_HTTPC_BODY_MAX - 1) copy_len = IDCU_HTTPC_BODY_MAX - 1;
                    memcpy(resp->body, body_start, copy_len);
                    resp->body_length = copy_len;
                }
            }
        } else if (has_content_length && resp->body_length < content_length) {
            size_t remaining = content_length - resp->body_length;
            size_t copy_len = total_received;
            if (copy_len > remaining) copy_len = remaining;
            if (resp->body_length + copy_len > IDCU_HTTPC_BODY_MAX - 1) {
                copy_len = IDCU_HTTPC_BODY_MAX - 1 - resp->body_length;
            }
            memcpy(resp->body + resp->body_length, buffer, copy_len);
            resp->body_length += copy_len;
            total_received = 0;
        }
    }

    resp->body[resp->body_length] = '\0';
    return IDCU_ERR_OK;
}

int idcu_http_client_execute(idcu_HttpClient* client, idcu_HttpClientRequest* req, idcu_HttpClientResponse* resp) {
    if (!client || !req || !resp) return IDCU_ERR_INVALID_PARAM;

    char host[IDCU_ADDR_MAX];
    uint16_t port;
    char path[IDCU_HTTPC_URL_MAX];

    int ret = parse_url(req->url, host, sizeof(host), &port, path, sizeof(path));
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to parse URL: %s", req->url);
        return ret;
    }

    if (client->connected) {
        if (strcmp(client->host, host) != 0 || client->port != port) {
            idcu_network_socket_close(&client->socket);
            client->connected = 0;
        }
    }

    if (!client->connected) {
        ret = idcu_network_socket_create(&client->socket, IDCU_NET_PROTO_TCP);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }

        ret = idcu_network_socket_connect(&client->socket, host, port);
        if (ret != IDCU_ERR_OK) {
            IDCU_LOG_ERROR("Failed to connect to %s:%d", host, port);
            idcu_network_socket_close(&client->socket);
            return ret;
        }

        strncpy(client->host, host, sizeof(client->host) - 1);
        client->host[sizeof(client->host) - 1] = '\0';
        client->port = port;
        client->connected = 1;
        IDCU_LOG_INFO("Connected to %s:%d", host, port);
    }

    char request_buffer[IDCU_HTTPC_RESPONSE_BUF_SIZE];
    size_t request_len = 0;
    ret = build_request_string(req, host, path, request_buffer, sizeof(request_buffer), &request_len);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    size_t sent = 0;
    ret = idcu_network_socket_send(&client->socket, request_buffer, request_len, &sent);
    if (ret != IDCU_ERR_OK || sent != request_len) {
        IDCU_LOG_ERROR("Failed to send request");
        idcu_network_socket_close(&client->socket);
        client->connected = 0;
        return IDCU_ERR_NETWORK_SEND;
    }

    ret = receive_response(client, resp);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to receive response");
    }

    idcu_network_socket_close(&client->socket);
    client->connected = 0;

    IDCU_LOG_INFO("HTTP %s %s - Status: %d", idcu_http_client_method_to_string(req->method), req->url, resp->status_code);

    return ret;
}

int idcu_http_client_get(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* resp) {
    idcu_HttpClientRequest req;
    int ret = idcu_http_client_request_init(&req, IDCU_HTTPC_METHOD_GET, url);
    if (ret != IDCU_ERR_OK) return ret;

    ret = idcu_http_client_execute(client, &req, resp);
    idcu_http_client_request_destroy(&req);
    return ret;
}

int idcu_http_client_post(idcu_HttpClient* client, const char* url, const char* body, size_t body_len, idcu_HttpClientResponse* resp) {
    idcu_HttpClientRequest req;
    int ret = idcu_http_client_request_init(&req, IDCU_HTTPC_METHOD_POST, url);
    if (ret != IDCU_ERR_OK) return ret;

    if (body && body_len > 0) {
        idcu_http_client_request_set_body(&req, body, body_len);
        idcu_http_client_request_add_header(&req, "Content-Type", "application/json");
    }

    ret = idcu_http_client_execute(client, &req, resp);
    idcu_http_client_request_destroy(&req);
    return ret;
}
