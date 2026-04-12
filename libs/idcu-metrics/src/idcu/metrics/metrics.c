#include <idcu/metrics/metrics.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>

static void metric_dtor(void* element) {
    idcu_Metric* metric = *(idcu_Metric**)element;
    if (metric) {
        switch (metric->type) {
            case IDCU_METRIC_TYPE_COUNTER:
                idcu_metrics_counter_destroy(metric->data.counter);
                break;
            case IDCU_METRIC_TYPE_GAUGE:
                idcu_metrics_gauge_destroy(metric->data.gauge);
                break;
            case IDCU_METRIC_TYPE_HISTOGRAM:
                idcu_metrics_histogram_destroy(metric->data.histogram);
                break;
        }
        free(metric);
    }
}

int idcu_metrics_labels_init(idcu_MetricLabels* labels) {
    if (!labels) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(labels, 0, sizeof(idcu_MetricLabels));
    return IDCU_ERR_OK;
}

int idcu_metrics_labels_add(idcu_MetricLabels* labels, const char* name, const char* value) {
    if (!labels || !name || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    if (labels->label_count >= IDCU_METRIC_LABELS_MAX) {
        return IDCU_ERR_OUT_OF_RANGE;
    }
    strncpy(labels->labels[labels->label_count].name, name, IDCU_METRIC_LABEL_KEY_MAX - 1);
    strncpy(labels->labels[labels->label_count].value, value, IDCU_METRIC_LABEL_VALUE_MAX - 1);
    labels->label_count++;
    return IDCU_ERR_OK;
}

void idcu_metrics_labels_clear(idcu_MetricLabels* labels) {
    if (!labels) {
        return;
    }
    memset(labels, 0, sizeof(idcu_MetricLabels));
}

int idcu_metrics_registry_init(idcu_MetricsRegistry* registry) {
    if (!registry) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(registry, 0, sizeof(idcu_MetricsRegistry));
    int ret = idcu_vector_init_with_dtor(&registry->metrics, sizeof(idcu_Metric*), 16, metric_dtor);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    ret = idcu_hash_map_init(&registry->metrics_by_name, 64, sizeof(idcu_Metric*));
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&registry->metrics);
        return ret;
    }
    ret = idcu_mutex_init(&registry->lock);
    if (ret != IDCU_ERR_OK) {
        idcu_hash_map_destroy(&registry->metrics_by_name);
        idcu_vector_destroy(&registry->metrics);
        return ret;
    }
    registry->initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_metrics_registry_destroy(idcu_MetricsRegistry* registry) {
    if (!registry) {
        return;
    }
    idcu_mutex_lock(&registry->lock);
    idcu_hash_map_destroy(&registry->metrics_by_name);
    idcu_vector_destroy(&registry->metrics);
    idcu_Mutex lock_copy = registry->lock;
    registry->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);
}

idcu_MetricCounter* idcu_metrics_counter_create(const char* name, const char* help, const idcu_MetricLabels* labels) {
    if (!name || !help) {
        return NULL;
    }
    idcu_MetricCounter* counter = (idcu_MetricCounter*)calloc(1, sizeof(idcu_MetricCounter));
    if (!counter) {
        return NULL;
    }
    strncpy(counter->name, name, IDCU_METRIC_NAME_MAX - 1);
    strncpy(counter->help, help, IDCU_METRIC_HELP_MAX - 1);
    if (labels) {
        memcpy(&counter->labels, labels, sizeof(idcu_MetricLabels));
    }
    idcu_mutex_init(&counter->lock);
    counter->value = 0;
    return counter;
}

void idcu_metrics_counter_destroy(idcu_MetricCounter* counter) {
    if (!counter) {
        return;
    }
    idcu_mutex_destroy(&counter->lock);
    free(counter);
}

void idcu_metrics_counter_inc(idcu_MetricCounter* counter) {
    if (!counter) {
        return;
    }
    idcu_mutex_lock(&counter->lock);
    counter->value++;
    idcu_mutex_unlock(&counter->lock);
}

void idcu_metrics_counter_add(idcu_MetricCounter* counter, uint64_t value) {
    if (!counter) {
        return;
    }
    idcu_mutex_lock(&counter->lock);
    counter->value += value;
    idcu_mutex_unlock(&counter->lock);
}

uint64_t idcu_metrics_counter_get(idcu_MetricCounter* counter) {
    if (!counter) {
        return 0;
    }
    idcu_mutex_lock(&counter->lock);
    uint64_t value = counter->value;
    idcu_mutex_unlock(&counter->lock);
    return value;
}

idcu_MetricGauge* idcu_metrics_gauge_create(const char* name, const char* help, const idcu_MetricLabels* labels) {
    if (!name || !help) {
        return NULL;
    }
    idcu_MetricGauge* gauge = (idcu_MetricGauge*)calloc(1, sizeof(idcu_MetricGauge));
    if (!gauge) {
        return NULL;
    }
    strncpy(gauge->name, name, IDCU_METRIC_NAME_MAX - 1);
    strncpy(gauge->help, help, IDCU_METRIC_HELP_MAX - 1);
    if (labels) {
        memcpy(&gauge->labels, labels, sizeof(idcu_MetricLabels));
    }
    idcu_mutex_init(&gauge->lock);
    gauge->value = 0.0;
    return gauge;
}

void idcu_metrics_gauge_destroy(idcu_MetricGauge* gauge) {
    if (!gauge) {
        return;
    }
    idcu_mutex_destroy(&gauge->lock);
    free(gauge);
}

void idcu_metrics_gauge_set(idcu_MetricGauge* gauge, double value) {
    if (!gauge) {
        return;
    }
    idcu_mutex_lock(&gauge->lock);
    gauge->value = value;
    idcu_mutex_unlock(&gauge->lock);
}

void idcu_metrics_gauge_inc(idcu_MetricGauge* gauge) {
    if (!gauge) {
        return;
    }
    idcu_mutex_lock(&gauge->lock);
    gauge->value++;
    idcu_mutex_unlock(&gauge->lock);
}

void idcu_metrics_gauge_dec(idcu_MetricGauge* gauge) {
    if (!gauge) {
        return;
    }
    idcu_mutex_lock(&gauge->lock);
    gauge->value--;
    idcu_mutex_unlock(&gauge->lock);
}

void idcu_metrics_gauge_add(idcu_MetricGauge* gauge, double value) {
    if (!gauge) {
        return;
    }
    idcu_mutex_lock(&gauge->lock);
    gauge->value += value;
    idcu_mutex_unlock(&gauge->lock);
}

void idcu_metrics_gauge_sub(idcu_MetricGauge* gauge, double value) {
    if (!gauge) {
        return;
    }
    idcu_mutex_lock(&gauge->lock);
    gauge->value -= value;
    idcu_mutex_unlock(&gauge->lock);
}

double idcu_metrics_gauge_get(idcu_MetricGauge* gauge) {
    if (!gauge) {
        return 0.0;
    }
    idcu_mutex_lock(&gauge->lock);
    double value = gauge->value;
    idcu_mutex_unlock(&gauge->lock);
    return value;
}

idcu_MetricHistogram* idcu_metrics_histogram_create(const char* name, const char* help, const idcu_MetricLabels* labels, const double* buckets, size_t bucket_count) {
    if (!name || !help || !buckets || bucket_count == 0 || bucket_count > IDCU_METRIC_BUCKETS_MAX) {
        return NULL;
    }
    idcu_MetricHistogram* histogram = (idcu_MetricHistogram*)calloc(1, sizeof(idcu_MetricHistogram));
    if (!histogram) {
        return NULL;
    }
    strncpy(histogram->name, name, IDCU_METRIC_NAME_MAX - 1);
    strncpy(histogram->help, help, IDCU_METRIC_HELP_MAX - 1);
    if (labels) {
        memcpy(&histogram->labels, labels, sizeof(idcu_MetricLabels));
    }
    memcpy(histogram->buckets, buckets, bucket_count * sizeof(double));
    histogram->bucket_count = bucket_count;
    idcu_mutex_init(&histogram->lock);
    return histogram;
}

void idcu_metrics_histogram_destroy(idcu_MetricHistogram* histogram) {
    if (!histogram) {
        return;
    }
    idcu_mutex_destroy(&histogram->lock);
    free(histogram);
}

void idcu_metrics_histogram_observe(idcu_MetricHistogram* histogram, double value) {
    if (!histogram) {
        return;
    }
    idcu_mutex_lock(&histogram->lock);
    histogram->sample_count++;
    histogram->sample_sum += value;
    for (size_t i = 0; i < histogram->bucket_count; i++) {
        if (value <= histogram->buckets[i]) {
            histogram->bucket_counts[i]++;
        }
    }
    idcu_mutex_unlock(&histogram->lock);
}

void idcu_metrics_histogram_get_counts(idcu_MetricHistogram* histogram, uint64_t* sample_count, double* sample_sum) {
    if (!histogram) {
        return;
    }
    idcu_mutex_lock(&histogram->lock);
    if (sample_count) {
        *sample_count = histogram->sample_count;
    }
    if (sample_sum) {
        *sample_sum = histogram->sample_sum;
    }
    idcu_mutex_unlock(&histogram->lock);
}

int idcu_metrics_registry_register_counter(idcu_MetricsRegistry* registry, idcu_MetricCounter* counter) {
    if (!registry || !registry->initialized || !counter) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_Metric* metric = (idcu_Metric*)malloc(sizeof(idcu_Metric));
    if (!metric) {
        return IDCU_ERR_MEMORY;
    }
    metric->type = IDCU_METRIC_TYPE_COUNTER;
    metric->data.counter = counter;
    idcu_mutex_lock(&registry->lock);
    int ret = idcu_vector_push_back(&registry->metrics, &metric);
    if (ret == IDCU_ERR_OK) {
        idcu_hash_map_set(&registry->metrics_by_name, counter->name, metric);
    }
    idcu_mutex_unlock(&registry->lock);
    if (ret != IDCU_ERR_OK) {
        free(metric);
    }
    return ret;
}

int idcu_metrics_registry_register_gauge(idcu_MetricsRegistry* registry, idcu_MetricGauge* gauge) {
    if (!registry || !registry->initialized || !gauge) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_Metric* metric = (idcu_Metric*)malloc(sizeof(idcu_Metric));
    if (!metric) {
        return IDCU_ERR_MEMORY;
    }
    metric->type = IDCU_METRIC_TYPE_GAUGE;
    metric->data.gauge = gauge;
    idcu_mutex_lock(&registry->lock);
    int ret = idcu_vector_push_back(&registry->metrics, &metric);
    if (ret == IDCU_ERR_OK) {
        idcu_hash_map_set(&registry->metrics_by_name, gauge->name, metric);
    }
    idcu_mutex_unlock(&registry->lock);
    if (ret != IDCU_ERR_OK) {
        free(metric);
    }
    return ret;
}

int idcu_metrics_registry_register_histogram(idcu_MetricsRegistry* registry, idcu_MetricHistogram* histogram) {
    if (!registry || !registry->initialized || !histogram) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_Metric* metric = (idcu_Metric*)malloc(sizeof(idcu_Metric));
    if (!metric) {
        return IDCU_ERR_MEMORY;
    }
    metric->type = IDCU_METRIC_TYPE_HISTOGRAM;
    metric->data.histogram = histogram;
    idcu_mutex_lock(&registry->lock);
    int ret = idcu_vector_push_back(&registry->metrics, &metric);
    if (ret == IDCU_ERR_OK) {
        idcu_hash_map_set(&registry->metrics_by_name, histogram->name, metric);
    }
    idcu_mutex_unlock(&registry->lock);
    if (ret != IDCU_ERR_OK) {
        free(metric);
    }
    return ret;
}

static int append_labels(char* buffer, size_t buffer_size, size_t* offset, const idcu_MetricLabels* labels) {
    if (!labels || labels->label_count == 0) {
        return IDCU_ERR_OK;
    }
    int written = snprintf(buffer + *offset, buffer_size - *offset, "{");
    if (written < 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    *offset += written;
    for (size_t i = 0; i < labels->label_count; i++) {
        if (i > 0) {
            written = snprintf(buffer + *offset, buffer_size - *offset, ",");
            if (written < 0) {
                return IDCU_ERR_INVALID_ARG;
            }
            *offset += written;
        }
        written = snprintf(buffer + *offset, buffer_size - *offset, "%s=\"%s\"",
                          labels->labels[i].name, labels->labels[i].value);
        if (written < 0) {
            return IDCU_ERR_INVALID_ARG;
        }
        *offset += written;
    }
    written = snprintf(buffer + *offset, buffer_size - *offset, "}");
    if (written < 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    *offset += written;
    return IDCU_ERR_OK;
}

int idcu_metrics_to_prometheus(const idcu_MetricsRegistry* registry, char* buffer, size_t buffer_size) {
    if (!registry || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    size_t offset = 0;
    IDCU_VECTOR_FOR_EACH(&registry->metrics, idcu_Metric, metric, i) {
        if (!metric) continue;
        switch (metric->type) {
            case IDCU_METRIC_TYPE_COUNTER: {
                idcu_MetricCounter* counter = metric->data.counter;
                int written = snprintf(buffer + offset, buffer_size - offset, "# HELP %s %s\n", counter->name, counter->help);
                if (written < 0) break;
                offset += written;
                written = snprintf(buffer + offset, buffer_size - offset, "# TYPE %s counter\n", counter->name);
                if (written < 0) break;
                offset += written;
                written = snprintf(buffer + offset, buffer_size - offset, "%s", counter->name);
                if (written < 0) break;
                offset += written;
                append_labels(buffer, buffer_size, &offset, &counter->labels);
                uint64_t value = idcu_metrics_counter_get(counter);
                written = snprintf(buffer + offset, buffer_size - offset, " %" PRIu64 "\n", value);
                if (written < 0) break;
                offset += written;
                break;
            }
            case IDCU_METRIC_TYPE_GAUGE: {
                idcu_MetricGauge* gauge = metric->data.gauge;
                int written = snprintf(buffer + offset, buffer_size - offset, "# HELP %s %s\n", gauge->name, gauge->help);
                if (written < 0) break;
                offset += written;
                written = snprintf(buffer + offset, buffer_size - offset, "# TYPE %s gauge\n", gauge->name);
                if (written < 0) break;
                offset += written;
                written = snprintf(buffer + offset, buffer_size - offset, "%s", gauge->name);
                if (written < 0) break;
                offset += written;
                append_labels(buffer, buffer_size, &offset, &gauge->labels);
                double value = idcu_metrics_gauge_get(gauge);
                written = snprintf(buffer + offset, buffer_size - offset, " %g\n", value);
                if (written < 0) break;
                offset += written;
                break;
            }
            case IDCU_METRIC_TYPE_HISTOGRAM: {
                idcu_MetricHistogram* histogram = metric->data.histogram;
                int written = snprintf(buffer + offset, buffer_size - offset, "# HELP %s %s\n", histogram->name, histogram->help);
                if (written < 0) break;
                offset += written;
                written = snprintf(buffer + offset, buffer_size - offset, "# TYPE %s histogram\n", histogram->name);
                if (written < 0) break;
                offset += written;
                idcu_mutex_lock(&histogram->lock);
                uint64_t sample_count = histogram->sample_count;
                double sample_sum = histogram->sample_sum;
                for (size_t b = 0; b < histogram->bucket_count; b++) {
                    written = snprintf(buffer + offset, buffer_size - offset, "%s_bucket", histogram->name);
                    if (written < 0) break;
                    offset += written;
                    written = snprintf(buffer + offset, buffer_size - offset, "{le=\"%g\"}", histogram->buckets[b]);
                    if (written < 0) break;
                    offset += written;
                    written = snprintf(buffer + offset, buffer_size - offset, " %" PRIu64 "\n", histogram->bucket_counts[b]);
                    if (written < 0) break;
                    offset += written;
                }
                idcu_mutex_unlock(&histogram->lock);
                written = snprintf(buffer + offset, buffer_size - offset, "%s_sum", histogram->name);
                if (written < 0) break;
                offset += written;
                append_labels(buffer, buffer_size, &offset, &histogram->labels);
                written = snprintf(buffer + offset, buffer_size - offset, " %g\n", sample_sum);
                if (written < 0) break;
                offset += written;
                written = snprintf(buffer + offset, buffer_size - offset, "%s_count", histogram->name);
                if (written < 0) break;
                offset += written;
                append_labels(buffer, buffer_size, &offset, &histogram->labels);
                written = snprintf(buffer + offset, buffer_size - offset, " %" PRIu64 "\n", sample_count);
                if (written < 0) break;
                offset += written;
                break;
            }
        }
    }
    return IDCU_ERR_OK;
}

int idcu_metrics_to_json(const idcu_MetricsRegistry* registry, char* buffer, size_t buffer_size) {
    if (!registry || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    size_t offset = 0;
    int written = snprintf(buffer + offset, buffer_size - offset, "[");
    if (written < 0) return IDCU_ERR_INVALID_ARG;
    offset += written;
    size_t metric_count = idcu_vector_size(&registry->metrics);
    IDCU_VECTOR_FOR_EACH(&registry->metrics, idcu_Metric, metric, i) {
        if (!metric) continue;
        if (i > 0) {
            written = snprintf(buffer + offset, buffer_size - offset, ",");
            if (written < 0) break;
            offset += written;
        }
        switch (metric->type) {
            case IDCU_METRIC_TYPE_COUNTER: {
                idcu_MetricCounter* counter = metric->data.counter;
                uint64_t value = idcu_metrics_counter_get(counter);
                written = snprintf(buffer + offset, buffer_size - offset,
                                  "{\"name\":\"%s\",\"type\":\"counter\",\"value\":%" PRIu64 "}",
                                  counter->name, value);
                break;
            }
            case IDCU_METRIC_TYPE_GAUGE: {
                idcu_MetricGauge* gauge = metric->data.gauge;
                double value = idcu_metrics_gauge_get(gauge);
                written = snprintf(buffer + offset, buffer_size - offset,
                                  "{\"name\":\"%s\",\"type\":\"gauge\",\"value\":%g}",
                                  gauge->name, value);
                break;
            }
            case IDCU_METRIC_TYPE_HISTOGRAM: {
                idcu_MetricHistogram* histogram = metric->data.histogram;
                uint64_t sample_count;
                double sample_sum;
                idcu_metrics_histogram_get_counts(histogram, &sample_count, &sample_sum);
                written = snprintf(buffer + offset, buffer_size - offset,
                                  "{\"name\":\"%s\",\"type\":\"histogram\",\"count\":%" PRIu64 ",\"sum\":%g}",
                                  histogram->name, sample_count, sample_sum);
                break;
            }
        }
        if (written < 0) break;
        offset += written;
    }
    written = snprintf(buffer + offset, buffer_size - offset, "]");
    if (written < 0) return IDCU_ERR_INVALID_ARG;
    return IDCU_ERR_OK;
}
