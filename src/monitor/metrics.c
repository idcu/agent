#include "monitor/metrics.h"
#include <string.h>
#include <stdio.h>

int metrics_init(MetricsCollector* collector)
{
    if (!collector) {
        return ERR_INVALID_PARAM;
    }

    memset(collector, 0, sizeof(MetricsCollector));
    return mutex_init(&collector->lock);
}

void metrics_destroy(MetricsCollector* collector)
{
    if (!collector) {
        return;
    }
    mutex_destroy(&collector->lock);
}

int metrics_register(MetricsCollector* collector, const char* name, const char* desc, MetricType type)
{
    if (!collector || !name) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&collector->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < collector->count; i++) {
        if (strcmp(collector->metrics[i].name, name) == 0) {
            mutex_unlock(&collector->lock);
            return ERR_ALREADY_EXISTS;
        }
    }

    if (collector->count >= MAX_METRICS) {
        mutex_unlock(&collector->lock);
        return ERR_QUEUE_FULL;
    }

    Metric* m = &collector->metrics[collector->count];
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

    mutex_unlock(&collector->lock);
    return ERR_SUCCESS;
}

static Metric* find_metric(MetricsCollector* collector, const char* name)
{
    for (uint32_t i = 0; i < collector->count; i++) {
        if (strcmp(collector->metrics[i].name, name) == 0) {
            return &collector->metrics[i];
        }
    }
    return NULL;
}

int metrics_inc(MetricsCollector* collector, const char* name, uint64_t value)
{
    if (!collector || !name) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&collector->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    Metric* m = find_metric(collector, name);
    if (!m) {
        mutex_unlock(&collector->lock);
        return ERR_NOT_FOUND;
    }

    if (m->type != METRIC_COUNTER) {
        mutex_unlock(&collector->lock);
        return ERR_INVALID_PARAM;
    }

    m->value += value;
    mutex_unlock(&collector->lock);
    return ERR_SUCCESS;
}

int metrics_set(MetricsCollector* collector, const char* name, uint64_t value)
{
    if (!collector || !name) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&collector->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    Metric* m = find_metric(collector, name);
    if (!m) {
        mutex_unlock(&collector->lock);
        return ERR_NOT_FOUND;
    }

    if (m->type != METRIC_GAUGE) {
        mutex_unlock(&collector->lock);
        return ERR_INVALID_PARAM;
    }

    m->value = value;
    mutex_unlock(&collector->lock);
    return ERR_SUCCESS;
}

int metrics_observe(MetricsCollector* collector, const char* name, uint64_t value)
{
    if (!collector || !name) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&collector->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    Metric* m = find_metric(collector, name);
    if (!m) {
        mutex_unlock(&collector->lock);
        return ERR_NOT_FOUND;
    }

    if (m->type != METRIC_HISTOGRAM) {
        mutex_unlock(&collector->lock);
        return ERR_INVALID_PARAM;
    }

    m->sum += value;
    m->count++;
    if (value < m->min) {
        m->min = value;
    }
    if (value > m->max) {
        m->max = value;
    }

    mutex_unlock(&collector->lock);
    return ERR_SUCCESS;
}

uint64_t metrics_get(MetricsCollector* collector, const char* name)
{
    if (!collector || !name) {
        return 0;
    }

    int ret = mutex_lock(&collector->lock);
    if (ret != ERR_SUCCESS) {
        return 0;
    }

    Metric* m = find_metric(collector, name);
    uint64_t value = m ? m->value : 0;

    mutex_unlock(&collector->lock);
    return value;
}

int metrics_export_text(MetricsCollector* collector, char* buffer, size_t buffer_size)
{
    if (!collector || !buffer) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&collector->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    size_t offset = 0;
    int len = 0;

    for (uint32_t i = 0; i < collector->count; i++) {
        Metric* m = &collector->metrics[i];
        
        switch (m->type) {
            case METRIC_COUNTER:
                len = snprintf(buffer + offset, buffer_size - offset,
                    "%s: %llu (counter) - %s\n",
                    m->name, (unsigned long long)m->value, m->desc);
                break;
            case METRIC_GAUGE:
                len = snprintf(buffer + offset, buffer_size - offset,
                    "%s: %llu (gauge) - %s\n",
                    m->name, (unsigned long long)m->value, m->desc);
                break;
            case METRIC_HISTOGRAM:
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

    mutex_unlock(&collector->lock);
    return ERR_SUCCESS;
}

int metrics_export_prometheus(MetricsCollector* collector, char* buffer, size_t buffer_size)
{
    if (!collector || !buffer) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&collector->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    size_t offset = 0;
    int len = 0;

    for (uint32_t i = 0; i < collector->count; i++) {
        Metric* m = &collector->metrics[i];
        
        switch (m->type) {
            case METRIC_COUNTER:
                len = snprintf(buffer + offset, buffer_size - offset,
                    "# HELP %s %s\n# TYPE %s counter\n%s %llu\n",
                    m->name, m->desc, m->name, m->name, (unsigned long long)m->value);
                break;
            case METRIC_GAUGE:
                len = snprintf(buffer + offset, buffer_size - offset,
                    "# HELP %s %s\n# TYPE %s gauge\n%s %llu\n",
                    m->name, m->desc, m->name, m->name, (unsigned long long)m->value);
                break;
            case METRIC_HISTOGRAM:
                if (m->count > 0) {
                    len = snprintf(buffer + offset, buffer_size - offset,
                        "# HELP %s %s\n# TYPE %s summary\n%s_sum %llu\n%s_count %llu\n",
                        m->name, m->desc, m->name,
                        m->name, (unsigned long long)m->sum,
                        m->name, (unsigned long long)m->count);
                }
                break;
        }
        
        if (len < 0 || (size_t)len >= buffer_size - offset) {
            break;
        }
        offset += len;
    }

    mutex_unlock(&collector->lock);
    return ERR_SUCCESS;
}
