#include "idcu/metrics/metrics.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static idcu_MetricsCollector g_global_collector;
static int g_global_initialized = 0;

int idcu_metrics_init(idcu_MetricsCollector* collector)
{
    if (!collector) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_hash_map_init(&collector->metrics_map, sizeof(idcu_Metric*), 16);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    ret = idcu_vector_init(&collector->metrics_list, sizeof(idcu_Metric*), 16);
    if (ret != IDCU_ERR_OK) {
        idcu_hash_map_destroy(&collector->metrics_map);
        return ret;
    }

    return idcu_mutex_init(&collector->lock);
}

void idcu_metrics_destroy(idcu_MetricsCollector* collector)
{
    if (!collector) {
        return;
    }

    idcu_mutex_lock(&collector->lock);

    IDCU_VECTOR_FOR_EACH(&collector->metrics_list, idcu_Metric*, metric, i) {
        free(*metric);
    }

    idcu_vector_destroy(&collector->metrics_list);
    idcu_hash_map_destroy(&collector->metrics_map);
    idcu_mutex_unlock(&collector->lock);
    idcu_mutex_destroy(&collector->lock);
}

int idcu_metrics_register(idcu_MetricsCollector* collector, const char* name, 
                           const char* desc, idcu_MetricType type)
{
    if (!collector || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    if (idcu_hash_map_contains(&collector->metrics_map, name)) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_ALREADY_EXISTS;
    }

    idcu_Metric* m = (idcu_Metric*)malloc(sizeof(idcu_Metric));
    if (!m) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_NO_MEMORY;
    }

    memset(m, 0, sizeof(idcu_Metric));
    strncpy(m->name, name, sizeof(m->name) - 1);
    m->name[sizeof(m->name) - 1] = '\0';

    if (desc) {
        strncpy(m->desc, desc, sizeof(m->desc) - 1);
        m->desc[sizeof(m->desc) - 1] = '\0';
    }

    m->type = type;
    m->value = 0;
    m->min = UINT64_MAX;
    m->max = 0;
    m->sum = 0;
    m->count = 0;

    idcu_hash_map_insert(&collector->metrics_map, name, &m);
    idcu_vector_push_back(&collector->metrics_list, &m);

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

static idcu_Metric* find_metric(idcu_MetricsCollector* collector, const char* name)
{
    idcu_Metric* m = NULL;
    if (idcu_hash_map_get(&collector->metrics_map, name, &m) == IDCU_ERR_OK) {
        return m;
    }
    return NULL;
}

int idcu_metrics_inc(idcu_MetricsCollector* collector, const char* name, uint64_t value)
{
    if (!collector || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    idcu_Metric* m = find_metric(collector, name);
    if (!m) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    if (m->type != IDCU_METRIC_COUNTER) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_INVALID_PARAM;
    }

    m->value += value;
    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_metrics_set(idcu_MetricsCollector* collector, const char* name, uint64_t value)
{
    if (!collector || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    idcu_Metric* m = find_metric(collector, name);
    if (!m) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    if (m->type != IDCU_METRIC_GAUGE) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_INVALID_PARAM;
    }

    m->value = value;
    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_metrics_observe(idcu_MetricsCollector* collector, const char* name, uint64_t value)
{
    if (!collector || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    idcu_Metric* m = find_metric(collector, name);
    if (!m) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    if (m->type != IDCU_METRIC_HISTOGRAM) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_INVALID_PARAM;
    }

    m->sum += value;
    m->count++;
    if (value < m->min) {
        m->min = value;
    }
    if (value > m->max) {
        m->max = value;
    }

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

uint64_t idcu_metrics_get(idcu_MetricsCollector* collector, const char* name)
{
    if (!collector || !name) {
        return 0;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }

    idcu_Metric* m = find_metric(collector, name);
    uint64_t value = m ? m->value : 0;

    idcu_mutex_unlock(&collector->lock);
    return value;
}

int idcu_metrics_export_text(idcu_MetricsCollector* collector, idcu_StringBuf* buf)
{
    if (!collector || !buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    IDCU_VECTOR_FOR_EACH(&collector->metrics_list, idcu_Metric*, metric_ptr, i) {
        idcu_Metric* m = *metric_ptr;

        switch (m->type) {
            case IDCU_METRIC_COUNTER:
                idcu_strbuf_append_format(buf, "%s: %llu (counter) - %s\n",
                    m->name, (unsigned long long)m->value, m->desc);
                break;
            case IDCU_METRIC_GAUGE:
                idcu_strbuf_append_format(buf, "%s: %llu (gauge) - %s\n",
                    m->name, (unsigned long long)m->value, m->desc);
                break;
            case IDCU_METRIC_HISTOGRAM:
                if (m->count > 0) {
                    idcu_strbuf_append_format(buf,
                        "%s: count=%llu sum=%llu min=%llu max=%llu (histogram) - %s\n",
                        m->name, (unsigned long long)m->count,
                        (unsigned long long)m->sum,
                        (unsigned long long)m->min,
                        (unsigned long long)m->max, m->desc);
                } else {
                    idcu_strbuf_append_format(buf, "%s: (histogram, no data) - %s\n",
                        m->name, m->desc);
                }
                break;
        }
    }

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_metrics_export_prometheus(idcu_MetricsCollector* collector, idcu_StringBuf* buf)
{
    if (!collector || !buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    IDCU_VECTOR_FOR_EACH(&collector->metrics_list, idcu_Metric*, metric_ptr, i) {
        idcu_Metric* m = *metric_ptr;

        switch (m->type) {
            case IDCU_METRIC_COUNTER:
                idcu_strbuf_append_format(buf,
                    "# HELP %s %s\n# TYPE %s counter\n%s %llu\n",
                    m->name, m->desc, m->name, m->name, (unsigned long long)m->value);
                break;
            case IDCU_METRIC_GAUGE:
                idcu_strbuf_append_format(buf,
                    "# HELP %s %s\n# TYPE %s gauge\n%s %llu\n",
                    m->name, m->desc, m->name, m->name, (unsigned long long)m->value);
                break;
            case IDCU_METRIC_HISTOGRAM:
                if (m->count > 0) {
                    idcu_strbuf_append_format(buf,
                        "# HELP %s %s\n# TYPE %s summary\n%s_sum %llu\n%s_count %llu\n",
                        m->name, m->desc, m->name,
                        m->name, (unsigned long long)m->sum,
                        m->name, (unsigned long long)m->count);
                } else {
                    idcu_strbuf_append_format(buf,
                        "# HELP %s %s\n# TYPE %s summary\n",
                        m->name, m->desc, m->name);
                }
                break;
        }
    }

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_global_metrics_init(void)
{
    if (g_global_initialized) {
        return IDCU_ERR_SUCCESS;
    }
    int ret = idcu_metrics_init(&g_global_collector);
    if (ret == IDCU_ERR_SUCCESS) {
        g_global_initialized = 1;
    }
    return ret;
}

void idcu_global_metrics_destroy(void)
{
    if (!g_global_initialized) {
        return;
    }
    idcu_metrics_destroy(&g_global_collector);
    g_global_initialized = 0;
}

idcu_MetricsCollector* idcu_global_metrics_collector(void)
{
    return g_global_initialized ? &g_global_collector : NULL;
}

int idcu_global_metrics_register(const char* name, const char* desc, idcu_MetricType type)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_register(&g_global_collector, name, desc, type);
}

int idcu_global_metrics_inc(const char* name, uint64_t value)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_inc(&g_global_collector, name, value);
}

int idcu_global_metrics_set(const char* name, uint64_t value)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_set(&g_global_collector, name, value);
}

int idcu_global_metrics_observe(const char* name, uint64_t value)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_observe(&g_global_collector, name, value);
}

uint64_t idcu_global_metrics_get(const char* name)
{
    if (!g_global_initialized) {
        return 0;
    }
    return idcu_metrics_get(&g_global_collector, name);
}

int idcu_global_metrics_export_text(idcu_StringBuf* buf)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_export_text(&g_global_collector, buf);
}

int idcu_global_metrics_export_prometheus(idcu_StringBuf* buf)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_export_prometheus(&g_global_collector, buf);
}
