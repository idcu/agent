#include "monitor/metrics.h"
#include <string.h>
#include <stdio.h>

int idcu_metrics_init(idcu_MetricsCollector* collector)
{
    if (!collector) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(collector, 0, sizeof(idcu_MetricsCollector));
    return idcu_mutex_init(&collector->lock);
}

void idcu_metrics_destroy(idcu_MetricsCollector* collector)
{
    if (!collector) {
        return;
    }
    idcu_mutex_destroy(&collector->lock);
}

int idcu_metrics_register(idcu_MetricsCollector* collector, const char* name, const char* desc, idcu_MetricType type)
{
    if (!collector || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < collector->count; i++) {
        if (strcmp(collector->metrics[i].name, name) == 0) {
            idcu_mutex_unlock(&collector->lock);
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }

    if (collector->count >= IDCU_MAX_METRICS) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_QUEUE_FULL;
    }

    idcu_Metric* m = &collector->metrics[collector->count];
    strncpy(m->name, name, sizeof(m->name) - 1);
    m->name[sizeof(m->name) - 1] = '\0';
    
    if (desc) {
        strncpy(m->desc, desc, sizeof(m->desc) - 1);
        m->desc[sizeof(m->desc) - 1] = '\0';
    } else {
        m->desc[0] = '\0';
    }
    
    m->type = type;
    m->value = 0;
    m->min = UINT64_MAX;
    m->max = 0;
    m->sum = 0;
    m->count = 0;
    collector->count++;

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

static idcu_Metric* find_metric(idcu_MetricsCollector* collector, const char* name)
{
    for (uint32_t i = 0; i < collector->count; i++) {
        if (strcmp(collector->metrics[i].name, name) == 0) {
            return &collector->metrics[i];
        }
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

int idcu_metrics_export_text(idcu_MetricsCollector* collector, char* buffer, size_t buffer_size)
{
    if (!collector || !buffer) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    size_t offset = 0;
    int len = 0;

    for (uint32_t i = 0; i < collector->count; i++) {
        idcu_Metric* m = &collector->metrics[i];
        
        switch (m->type) {
            case IDCU_METRIC_COUNTER:
                len = snprintf(buffer + offset, buffer_size - offset,
                    "%s: %llu (counter) - %s\n",
                    m->name, (unsigned long long)m->value, m->desc);
                break;
            case IDCU_METRIC_GAUGE:
                len = snprintf(buffer + offset, buffer_size - offset,
                    "%s: %llu (gauge) - %s\n",
                    m->name, (unsigned long long)m->value, m->desc);
                break;
            case IDCU_METRIC_HISTOGRAM:
                if (m->count > 0) {
                    len = snprintf(buffer + offset, buffer_size - offset,
                        "%s: count=%llu sum=%llu min=%llu max=%llu (histogram) - %s\n",
                        m->name, (unsigned long long)m->count,
                        (unsigned long long)m->sum,
                        (unsigned long long)m->min,
                        (unsigned long long)m->max, m->desc);
                } else {
                    len = snprintf(buffer + offset, buffer_size - offset,
                        "%s: (histogram, no data) - %s\n",
                        m->name, m->desc);
                }
                break;
        }
        
        if (len < 0 || (size_t)len >= buffer_size - offset) {
            break;
        }
        offset += len;
    }

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_metrics_export_prometheus(idcu_MetricsCollector* collector, char* buffer, size_t buffer_size)
{
    if (!collector || !buffer) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&collector->lock);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    size_t offset = 0;
    int len = 0;

    for (uint32_t i = 0; i < collector->count; i++) {
        idcu_Metric* m = &collector->metrics[i];
        
        switch (m->type) {
            case IDCU_METRIC_COUNTER:
                len = snprintf(buffer + offset, buffer_size - offset,
                    "# HELP %s %s\n# TYPE %s counter\n%s %llu\n",
                    m->name, m->desc, m->name, m->name, (unsigned long long)m->value);
                break;
            case IDCU_METRIC_GAUGE:
                len = snprintf(buffer + offset, buffer_size - offset,
                    "# HELP %s %s\n# TYPE %s gauge\n%s %llu\n",
                    m->name, m->desc, m->name, m->name, (unsigned long long)m->value);
                break;
            case IDCU_METRIC_HISTOGRAM:
                if (m->count > 0) {
                    len = snprintf(buffer + offset, buffer_size - offset,
                        "# HELP %s %s\n# TYPE %s summary\n%s_sum %llu\n%s_count %llu\n",
                        m->name, m->desc, m->name,
                        m->name, (unsigned long long)m->sum,
                        m->name, (unsigned long long)m->count);
                } else {
                    len = snprintf(buffer + offset, buffer_size - offset,
                        "# HELP %s %s\n# TYPE %s summary\n",
                        m->name, m->desc, m->name);
                }
                break;
        }
        
        if (len < 0) {
            break;
        }
        if ((size_t)len >= buffer_size - offset) {
            idcu_mutex_unlock(&collector->lock);
            return IDCU_ERR_BUFFER_TOO_SMALL;
        }
        offset += len;
    }

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_OK;
}
