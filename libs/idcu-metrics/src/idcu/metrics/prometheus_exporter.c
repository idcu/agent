#include "idcu/metrics/prometheus_exporter.h"
#include "idcu/common/string_buf.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int idcu_prometheus_exporter_init(idcu_PrometheusExporter *exporter, idcu_MetricsCollector *metrics,
                                  const char *bind_address, uint16_t port) {
    if (!exporter || !metrics) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(exporter, 0, sizeof(idcu_PrometheusExporter));
    exporter->metrics = metrics;
    exporter->running = 0;

    if (bind_address && strlen(bind_address) > 0) {
        strncpy(exporter->bind_address, bind_address, sizeof(exporter->bind_address) - 1);
    } else {
        strncpy(exporter->bind_address, IDCU_PROMETHEUS_DEFAULT_ADDR,
                sizeof(exporter->bind_address) - 1);
    }
    exporter->bind_address[sizeof(exporter->bind_address) - 1] = '\0';

    exporter->port = (port > 0) ? port : IDCU_PROMETHEUS_DEFAULT_PORT;

    int ret = idcu_mutex_init(&exporter->lock);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    return IDCU_ERR_OK;
}

void idcu_prometheus_exporter_destroy(idcu_PrometheusExporter *exporter) {
    if (!exporter) {
        return;
    }

    idcu_prometheus_exporter_stop(exporter);
    idcu_mutex_destroy(&exporter->lock);
}

int idcu_prometheus_exporter_start(idcu_PrometheusExporter *exporter) {
    if (!exporter) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&exporter->lock);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    if (exporter->running) {
        idcu_mutex_unlock(&exporter->lock);
        return IDCU_ERR_OK;
    }

    ret = idcu_network_server_create(&exporter->server, IDCU_NET_PROTO_TCP, exporter->bind_address,
                                     exporter->port);
    if (ret != IDCU_ERR_OK) {
        idcu_mutex_unlock(&exporter->lock);
        IDCU_LOG_ERROR("Failed to create Prometheus exporter server");
        return ret;
    }

    ret = idcu_network_server_listen(&exporter->server);
    if (ret != IDCU_ERR_OK) {
        idcu_network_server_destroy(&exporter->server);
        idcu_mutex_unlock(&exporter->lock);
        IDCU_LOG_ERROR("Failed to listen on Prometheus exporter port");
        return ret;
    }

    exporter->running = 1;
    idcu_mutex_unlock(&exporter->lock);

    IDCU_LOG_INFO("Prometheus exporter started on %s:%d", exporter->bind_address, exporter->port);
    return IDCU_ERR_OK;
}

int idcu_prometheus_exporter_stop(idcu_PrometheusExporter *exporter) {
    if (!exporter) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&exporter->lock);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    if (!exporter->running) {
        idcu_mutex_unlock(&exporter->lock);
        return IDCU_ERR_OK;
    }

    idcu_network_server_destroy(&exporter->server);
    exporter->running = 0;
    idcu_mutex_unlock(&exporter->lock);

    IDCU_LOG_INFO("Prometheus exporter stopped");
    return IDCU_ERR_OK;
}

static int is_metrics_path(const char *request) {
    return strstr(request, IDCU_PROMETHEUS_METRICS_PATH) != NULL;
}

int idcu_prometheus_exporter_handle_request(idcu_PrometheusExporter *exporter,
                                            idcu_NetworkSocket *client) {
    if (!exporter || !client) {
        return IDCU_ERR_INVALID_PARAM;
    }

    char request_buffer[4096];
    size_t received = 0;
    int ret =
        idcu_network_socket_recv(client, request_buffer, sizeof(request_buffer) - 1, &received);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    request_buffer[received] = '\0';

    char response[IDCU_PROMETHEUS_BUFFER_SIZE];
    size_t response_len = 0;

    if (is_metrics_path(request_buffer)) {
        idcu_StringBuf buf;
        ret = idcu_strbuf_init(&buf, IDCU_PROMETHEUS_BUFFER_SIZE - 512);
        if (ret != IDCU_ERR_OK) {
            snprintf(response, sizeof(response),
                     "HTTP/1.1 500 Internal Server Error\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 21\r\n"
                     "\r\n"
                     "Failed to export metrics");
            response_len = strlen(response);
        } else {
            ret = idcu_metrics_export_prometheus(exporter->metrics, &buf);
            if (ret != IDCU_ERR_OK) {
                idcu_strbuf_destroy(&buf);
                snprintf(response, sizeof(response),
                         "HTTP/1.1 500 Internal Server Error\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Length: 21\r\n"
                         "\r\n"
                         "Failed to export metrics");
                response_len = strlen(response);
            } else {
                size_t metrics_len = idcu_strbuf_size(&buf);
                snprintf(response, sizeof(response),
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/plain; version=0.0.4\r\n"
                         "Content-Length: %zu\r\n"
                         "\r\n"
                         "%s",
                         metrics_len, idcu_strbuf_data(&buf));
                response_len = strlen(response);
            }
            idcu_strbuf_destroy(&buf);
        }
    } else {
        snprintf(response, sizeof(response),
                 "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: 9\r\n"
                 "\r\n"
                 "Not Found");
        response_len = strlen(response);
    }

    size_t sent = 0;
    ret = idcu_network_socket_send(client, response, response_len, &sent);
    idcu_network_socket_close(client);

    return ret;
}

int idcu_prometheus_exporter_poll(idcu_PrometheusExporter *exporter, int timeout_ms) {
    (void)timeout_ms;

    if (!exporter) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&exporter->lock);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    if (!exporter->running) {
        idcu_mutex_unlock(&exporter->lock);
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_NetworkSocket client;
    ret = idcu_network_server_accept(&exporter->server, &client);
    idcu_mutex_unlock(&exporter->lock);

    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    if (client.fd != IDCU_INVALID_SOCKET) {
        return idcu_prometheus_exporter_handle_request(exporter, &client);
    }

    return IDCU_ERR_OK;
}
