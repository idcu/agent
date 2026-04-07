#include "idcu/metrics/metrics.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static idcu_MetricsCollector g_global_collector;
static int g_global_initialized = 0;

static const char* g_grafana_dashboard_json = 
"{\n"
"  \"annotations\": {\n"
"    \"list\": []\n"
"  },\n"
"  \"editable\": true,\n"
"  \"fiscalYearStartMonth\": 1,\n"
"  \"graphTooltip\": 0,\n"
"  \"id\": null,\n"
"  \"links\": [],\n"
"  \"liveNow\": false,\n"
"  \"panels\": [\n"
"    {\n"
"      \"datasource\": {\n"
"        \"type\": \"prometheus\",\n"
"        \"uid\": \"${DS_PROMETHEUS}\"\n"
"      },\n"
"      \"fieldConfig\": {\n"
"        \"defaults\": {\n"
"          \"color\": {\n"
"            \"mode\": \"palette-classic\"\n"
"          },\n"
"          \"mappings\": [],\n"
"          \"thresholds\": {\n"
"            \"mode\": \"absolute\",\n"
"            \"steps\": [\n"
"              {\n"
"                \"color\": \"green\",\n"
"                \"value\": null\n"
"              },\n"
"              {\n"
"                \"color\": \"red\",\n"
"                \"value\": 80\n"
"              }\n"
"            ]\n"
"          }\n"
"        },\n"
"        \"overrides\": []\n"
"      },\n"
"      \"gridPos\": {\n"
"        \"h\": 4,\n"
"        \"w\": 6,\n"
"        \"x\": 0,\n"
"        \"y\": 0\n"
"      },\n"
"      \"id\": 1,\n"
"      \"options\": {\n"
"        \"colorMode\": \"value\",\n"
"        \"graphMode\": \"area\",\n"
"        \"justifyMode\": \"auto\",\n"
"        \"orientation\": \"auto\",\n"
"        \"reduceOptions\": {\n"
"          \"calcs\": [\"lastNotNull\"],\n"
"          \"fields\": \"\",\n"
"          \"values\": false\n"
"        },\n"
"        \"textMode\": \"auto\"\n"
"      },\n"
"      \"pluginVersion\": \"9.0.0\",\n"
"      \"targets\": [\n"
"        {\n"
"          \"expr\": \"idcu_nodes_online\",\n"
"          \"legendFormat\": \"Nodes Online\",\n"
"          \"refId\": \"A\"\n"
"        }\n"
"      ],\n"
"      \"title\": \"Nodes Online\",\n"
"      \"type\": \"stat\"\n"
"    },\n"
"    {\n"
"      \"datasource\": {\n"
"        \"type\": \"prometheus\",\n"
"        \"uid\": \"${DS_PROMETHEUS}\"\n"
"      },\n"
"      \"fieldConfig\": {\n"
"        \"defaults\": {\n"
"          \"color\": {\n"
"            \"mode\": \"palette-classic\"\n"
"          },\n"
"          \"mappings\": [],\n"
"          \"thresholds\": {\n"
"            \"mode\": \"absolute\",\n"
"            \"steps\": [\n"
"              {\n"
"                \"color\": \"green\",\n"
"                \"value\": null\n"
"              },\n"
"              {\n"
"                \"color\": \"yellow\",\n"
"                \"value\": 50\n"
"              },\n"
"              {\n"
"                \"color\": \"red\",\n"
"                \"value\": 100\n"
"              }\n"
"            ]\n"
"          },\n"
"          \"unit\": \"ms\"\n"
"        },\n"
"        \"overrides\": []\n"
"      },\n"
"      \"gridPos\": {\n"
"        \"h\": 4,\n"
"        \"w\": 6,\n"
"        \"x\": 6,\n"
"        \"y\": 0\n"
"      },\n"
"      \"id\": 2,\n"
"      \"options\": {\n"
"        \"colorMode\": \"value\",\n"
"        \"graphMode\": \"area\",\n"
"        \"justifyMode\": \"auto\",\n"
"        \"orientation\": \"auto\",\n"
"        \"reduceOptions\": {\n"
"          \"calcs\": [\"lastNotNull\"],\n"
"          \"fields\": \"\",\n"
"          \"values\": false\n"
"        },\n"
"        \"textMode\": \"auto\"\n"
"      },\n"
"      \"pluginVersion\": \"9.0.0\",\n"
"      \"targets\": [\n"
"        {\n"
"          \"expr\": \"idcu_avg_latency_ms\",\n"
"          \"legendFormat\": \"Avg Latency\",\n"
"          \"refId\": \"A\"\n"
"        }\n"
"      ],\n"
"      \"title\": \"Average Latency\",\n"
"      \"type\": \"stat\"\n"
"    },\n"
"    {\n"
"      \"datasource\": {\n"
"        \"type\": \"prometheus\",\n"
"        \"uid\": \"${DS_PROMETHEUS}\"\n"
"      },\n"
"      \"fieldConfig\": {\n"
"        \"defaults\": {\n"
"          \"color\": {\n"
"            \"mode\": \"palette-classic\"\n"
"          },\n"
"          \"mappings\": [],\n"
"          \"thresholds\": {\n"
"            \"mode\": \"absolute\",\n"
"            \"steps\": [\n"
"              {\n"
"                \"color\": \"green\",\n"
"                \"value\": null\n"
"              }\n"
"            ]\n"
"          }\n"
"        },\n"
"        \"overrides\": []\n"
"      },\n"
"      \"gridPos\": {\n"
"        \"h\": 4,\n"
"        \"w\": 6,\n"
"        \"x\": 12,\n"
"        \"y\": 0\n"
"      },\n"
"      \"id\": 3,\n"
"      \"options\": {\n"
"        \"colorMode\": \"value\",\n"
"        \"graphMode\": \"area\",\n"
"        \"justifyMode\": \"auto\",\n"
"        \"orientation\": \"auto\",\n"
"        \"reduceOptions\": {\n"
"          \"calcs\": [\"lastNotNull\"],\n"
"          \"fields\": \"\",\n"
"          \"values\": false\n"
"        },\n"
"        \"textMode\": \"auto\"\n"
"      },\n"
"      \"pluginVersion\": \"9.0.0\",\n"
"      \"targets\": [\n"
"        {\n"
"          \"expr\": \"rate(idcu_total_messages_sent[5m])\",\n"
"          \"legendFormat\": \"Msgs/Sec\",\n"
"          \"refId\": \"A\"\n"
"        }\n"
"      ],\n"
"      \"title\": \"Message Rate\",\n"
"      \"type\": \"stat\"\n"
"    },\n"
"    {\n"
"      \"datasource\": {\n"
"        \"type\": \"prometheus\",\n"
"        \"uid\": \"${DS_PROMETHEUS}\"\n"
"      },\n"
"      \"fieldConfig\": {\n"
"        \"defaults\": {\n"
"          \"color\": {\n"
"            \"mode\": \"palette-classic\"\n"
"          },\n"
"          \"mappings\": [],\n"
"          \"thresholds\": {\n"
"            \"mode\": \"absolute\",\n"
"            \"steps\": [\n"
"              {\n"
"                \"color\": \"green\",\n"
"                \"value\": null\n"
"              },\n"
"              {\n"
"                \"color\": \"red\",\n"
"                \"value\": 0.9\n"
"              }\n"
"            ]\n"
"          },\n"
"          \"unit\": \"percent\"\n"
"        },\n"
"        \"overrides\": []\n"
"      },\n"
"      \"gridPos\": {\n"
"        \"h\": 4,\n"
"        \"w\": 6,\n"
"        \"x\": 18,\n"
"        \"y\": 0\n"
"      },\n"
"      \"id\": 4,\n"
"      \"options\": {\n"
"        \"colorMode\": \"value\",\n"
"        \"graphMode\": \"area\",\n"
"        \"justifyMode\": \"auto\",\n"
"        \"orientation\": \"auto\",\n"
"        \"reduceOptions\": {\n"
"          \"calcs\": [\"lastNotNull\"],\n"
"          \"fields\": \"\",\n"
"          \"values\": false\n"
"        },\n"
"        \"textMode\": \"auto\"\n"
"      },\n"
"      \"pluginVersion\": \"9.0.0\",\n"
"      \"targets\": [\n"
"        {\n"
"          \"expr\": \"idcu_online_rate * 100\",\n"
"          \"legendFormat\": \"Online Rate\",\n"
"          \"refId\": \"A\"\n"
"        }\n"
"      ],\n"
"      \"title\": \"Node Online Rate\",\n"
"      \"type\": \"stat\"\n"
"    },\n"
"    {\n"
"      \"datasource\": {\n"
"        \"type\": \"prometheus\",\n"
"        \"uid\": \"${DS_PROMETHEUS}\"\n"
"      },\n"
"      \"fieldConfig\": {\n"
"        \"defaults\": {\n"
"          \"color\": {\n"
"            \"mode\": \"palette-classic\"\n"
"          },\n"
"          \"custom\": {\n"
"            \"axisCenteredZero\": false,\n"
"            \"axisColorMode\": \"text\",\n"
"            \"axisLabel\": \"\",\n"
"            \"axisPlacement\": \"auto\",\n"
"            \"barAlignment\": 0,\n"
"            \"drawStyle\": \"line\",\n"
"            \"fillOpacity\": 10,\n"
"            \"gradientMode\": \"none\",\n"
"            \"hideFrom\": {\n"
"              \"legend\": false,\n"
"              \"tooltip\": false,\n"
"              \"viz\": false\n"
"            },\n"
"            \"lineInterpolation\": \"linear\",\n"
"            \"lineWidth\": 1,\n"
"            \"pointSize\": 5,\n"
"            \"scaleDistribution\": {\n"
"              \"type\": \"linear\"\n"
"            },\n"
"            \"showPoints\": \"auto\",\n"
"            \"spanNulls\": false,\n"
"            \"stacking\": {\n"
"              \"group\": \"A\",\n"
"              \"mode\": \"none\"\n"
"            },\n"
"            \"thresholdsStyle\": {\n"
"              \"mode\": \"off\"\n"
"            }\n"
"          },\n"
"          \"mappings\": [],\n"
"          \"thresholds\": {\n"
"            \"mode\": \"absolute\",\n"
"            \"steps\": [\n"
"              {\n"
"                \"color\": \"green\",\n"
"                \"value\": null\n"
"              }\n"
"            ]\n"
"          }\n"
"        },\n"
"        \"overrides\": []\n"
"      },\n"
"      \"gridPos\": {\n"
"        \"h\": 8,\n"
"        \"w\": 12,\n"
"        \"x\": 0,\n"
"        \"y\": 4\n"
"      },\n"
"      \"id\": 5,\n"
"      \"options\": {\n"
"        \"legend\": {\n"
"          \"calcs\": [],\n"
"          \"displayMode\": \"list\",\n"
"          \"placement\": \"bottom\",\n"
"          \"showLegend\": true\n"
"        },\n"
"        \"tooltip\": {\n"
"          \"mode\": \"single\",\n"
"          \"sort\": \"none\"\n"
"        }\n"
"      },\n"
"      \"targets\": [\n"
"        {\n"
"          \"expr\": \"idcu_total_messages_sent\",\n"
"          \"legendFormat\": \"Messages Sent\",\n"
"          \"refId\": \"A\"\n"
"        },\n"
"        {\n"
"          \"expr\": \"idcu_total_messages_received\",\n"
"          \"legendFormat\": \"Messages Received\",\n"
"          \"refId\": \"B\"\n"
"        }\n"
"      ],\n"
"      \"title\": \"Message Volume\",\n"
"      \"type\": \"timeseries\"\n"
"    },\n"
"    {\n"
"      \"datasource\": {\n"
"        \"type\": \"prometheus\",\n"
"        \"uid\": \"${DS_PROMETHEUS}\"\n"
"      },\n"
"      \"fieldConfig\": {\n"
"        \"defaults\": {\n"
"          \"color\": {\n"
"            \"mode\": \"palette-classic\"\n"
"          },\n"
"          \"custom\": {\n"
"            \"axisCenteredZero\": false,\n"
"            \"axisColorMode\": \"text\",\n"
"            \"axisLabel\": \"\",\n"
"            \"axisPlacement\": \"auto\",\n"
"            \"barAlignment\": 0,\n"
"            \"drawStyle\": \"line\",\n"
"            \"fillOpacity\": 10,\n"
"            \"gradientMode\": \"none\",\n"
"            \"hideFrom\": {\n"
"              \"legend\": false,\n"
"              \"tooltip\": false,\n"
"              \"viz\": false\n"
"            },\n"
"            \"lineInterpolation\": \"linear\",\n"
"            \"lineWidth\": 1,\n"
"            \"pointSize\": 5,\n"
"            \"scaleDistribution\": {\n"
"              \"type\": \"linear\"\n"
"            },\n"
"            \"showPoints\": \"auto\",\n"
"            \"spanNulls\": false,\n"
"            \"stacking\": {\n"
"              \"group\": \"A\",\n"
"              \"mode\": \"none\"\n"
"            },\n"
"            \"thresholdsStyle\": {\n"
"              \"mode\": \"off\"\n"
"            }\n"
"          },\n"
"          \"mappings\": [],\n"
"          \"thresholds\": {\n"
"            \"mode\": \"absolute\",\n"
"            \"steps\": [\n"
"              {\n"
"                \"color\": \"green\",\n"
"                \"value\": null\n"
"              }\n"
"            ]\n"
"          },\n"
"          \"unit\": \"bytes\"\n"
"        },\n"
"        \"overrides\": []\n"
"      },\n"
"      \"gridPos\": {\n"
"        \"h\": 8,\n"
"        \"w\": 12,\n"
"        \"x\": 12,\n"
"        \"y\": 4\n"
"      },\n"
"      \"id\": 6,\n"
"      \"options\": {\n"
"        \"legend\": {\n"
"          \"calcs\": [],\n"
"          \"displayMode\": \"list\",\n"
"          \"placement\": \"bottom\",\n"
"          \"showLegend\": true\n"
"        },\n"
"        \"tooltip\": {\n"
"          \"mode\": \"single\",\n"
"          \"sort\": \"none\"\n"
"        }\n"
"      },\n"
"      \"targets\": [\n"
"        {\n"
"          \"expr\": \"idcu_total_bytes_sent\",\n"
"          \"legendFormat\": \"Bytes Sent\",\n"
"          \"refId\": \"A\"\n"
"        },\n"
"        {\n"
"          \"expr\": \"idcu_total_bytes_received\",\n"
"          \"legendFormat\": \"Bytes Received\",\n"
"          \"refId\": \"B\"\n"
"        }\n"
"      ],\n"
"      \"title\": \"Network Traffic\",\n"
"      \"type\": \"timeseries\"\n"
"    }\n"
"  ],\n"
"  \"refresh\": \"5s\",\n"
"  \"schemaVersion\": 37,\n"
"  \"style\": \"dark\",\n"
"  \"tags\": [\"idcu\", \"distributed\"],\n"
"  \"templating\": {\n"
"    \"list\": []\n"
"  },\n"
"  \"time\": {\n"
"    \"from\": \"now-1h\",\n"
"    \"to\": \"now\"\n"
"  },\n"
"  \"timepicker\": {},\n"
"  \"timezone\": \"\",\n"
"  \"title\": \"IDCU Distributed System Dashboard\",\n"
"  \"uid\": \"idcu-distributed-dashboard\",\n"
"  \"version\": 1,\n"
"  \"weekStart\": \"\"\n"
"}\n";

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

    memset(collector->namespace_prefix, 0, sizeof(collector->namespace_prefix));
    
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

int idcu_metrics_set_namespace(idcu_MetricsCollector* collector, const char* ns)
{
    if (!collector) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (ns) {
        strncpy(collector->namespace_prefix, ns, sizeof(collector->namespace_prefix) - 1);
        collector->namespace_prefix[sizeof(collector->namespace_prefix) - 1] = '\0';
    } else {
        collector->namespace_prefix[0] = '\0';
    }
    
    return IDCU_ERR_OK;
}

static idcu_Metric* find_metric(idcu_MetricsCollector* collector, const char* name)
{
    idcu_Metric* m = NULL;
    if (idcu_hash_map_get(&collector->metrics_map, name, &m) == IDCU_ERR_OK) {
        return m;
    }
    return NULL;
}

int idcu_metrics_register(idcu_MetricsCollector* collector, const char* name, 
                           const char* desc, idcu_MetricType type)
{
    return idcu_metrics_register_with_labels(collector, name, desc, type, NULL, 0);
}

int idcu_metrics_register_with_labels(idcu_MetricsCollector* collector, const char* name, 
                                       const char* desc, idcu_MetricType type,
                                       const idcu_MetricLabel* labels, int label_count)
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
    m->label_count = 0;
    m->bucket_count = 0;
    
    if (labels && label_count > 0) {
        int copy_count = label_count < IDCU_METRIC_MAX_LABELS ? label_count : IDCU_METRIC_MAX_LABELS;
        for (int i = 0; i < copy_count; i++) {
            m->labels[i] = labels[i];
        }
        m->label_count = copy_count;
    }

    idcu_hash_map_insert(&collector->metrics_map, name, &m);
    idcu_vector_push_back(&collector->metrics_list, &m);

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_metrics_set_histogram_buckets(idcu_MetricsCollector* collector, const char* name,
                                        const uint64_t* buckets, int bucket_count)
{
    if (!collector || !name || !buckets || bucket_count <= 0) {
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

    int copy_count = bucket_count < IDCU_HISTOGRAM_MAX_BUCKETS ? bucket_count : IDCU_HISTOGRAM_MAX_BUCKETS;
    for (int i = 0; i < copy_count; i++) {
        m->buckets[i] = buckets[i];
        m->bucket_counts[i] = 0;
    }
    m->bucket_count = copy_count;

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_metrics_add_label(idcu_MetricsCollector* collector, const char* name,
                            const char* key, const char* value)
{
    if (!collector || !name || !key || !value) {
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

    if (m->label_count >= IDCU_METRIC_MAX_LABELS) {
        idcu_mutex_unlock(&collector->lock);
        return IDCU_ERR_NO_MEMORY;
    }

    strncpy(m->labels[m->label_count].key, key, IDCU_METRIC_LABEL_KEY_MAX - 1);
    m->labels[m->label_count].key[IDCU_METRIC_LABEL_KEY_MAX - 1] = '\0';
    strncpy(m->labels[m->label_count].value, value, IDCU_METRIC_LABEL_VALUE_MAX - 1);
    m->labels[m->label_count].value[IDCU_METRIC_LABEL_VALUE_MAX - 1] = '\0';
    m->label_count++;

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
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

int idcu_metrics_dec(idcu_MetricsCollector* collector, const char* name, uint64_t value)
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

    if (m->value >= value) {
        m->value -= value;
    } else {
        m->value = 0;
    }
    
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

    if (m->type != IDCU_METRIC_HISTOGRAM && m->type != IDCU_METRIC_SUMMARY) {
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
    
    if (m->type == IDCU_METRIC_HISTOGRAM && m->bucket_count > 0) {
        for (int i = 0; i < m->bucket_count; i++) {
            if (value <= m->buckets[i]) {
                m->bucket_counts[i]++;
            }
        }
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

static void append_labels(idcu_StringBuf* buf, const idcu_MetricLabel* labels, int label_count)
{
    if (label_count <= 0) {
        return;
    }
    
    idcu_strbuf_append(buf, "{");
    for (int i = 0; i < label_count; i++) {
        if (i > 0) {
            idcu_strbuf_append(buf, ",");
        }
        idcu_strbuf_append_format(buf, "%s=\"%s\"", labels[i].key, labels[i].value);
    }
    idcu_strbuf_append(buf, "}");
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
            case IDCU_METRIC_SUMMARY:
                if (m->count > 0) {
                    idcu_strbuf_append_format(buf,
                        "%s: count=%llu sum=%llu min=%llu max=%llu (histogram/summary) - %s\n",
                        m->name, (unsigned long long)m->count,
                        (unsigned long long)m->sum,
                        (unsigned long long)m->min,
                        (unsigned long long)m->max, m->desc);
                } else {
                    idcu_strbuf_append_format(buf, "%s: (histogram/summary, no data) - %s\n",
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
    return idcu_metrics_export_prometheus_with_timestamp(collector, buf, 0);
}

int idcu_metrics_export_prometheus_with_timestamp(idcu_MetricsCollector* collector, 
                                                    idcu_StringBuf* buf, uint64_t timestamp_ms)
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
        const char* full_name = m->name;
        
        if (collector->namespace_prefix[0] != '\0') {
            idcu_strbuf_append_format(buf, "# HELP %s_%s %s\n", 
                                      collector->namespace_prefix, m->name, m->desc);
        } else {
            idcu_strbuf_append_format(buf, "# HELP %s %s\n", m->name, m->desc);
        }

        switch (m->type) {
            case IDCU_METRIC_COUNTER:
                if (collector->namespace_prefix[0] != '\0') {
                    idcu_strbuf_append_format(buf, "# TYPE %s_%s counter\n", 
                                              collector->namespace_prefix, m->name);
                    idcu_strbuf_append_format(buf, "%s_%s", collector->namespace_prefix, m->name);
                } else {
                    idcu_strbuf_append_format(buf, "# TYPE %s counter\n", m->name);
                    idcu_strbuf_append(buf, m->name);
                }
                append_labels(buf, m->labels, m->label_count);
                idcu_strbuf_append_format(buf, " %llu", (unsigned long long)m->value);
                if (timestamp_ms > 0) {
                    idcu_strbuf_append_format(buf, " %llu", (unsigned long long)timestamp_ms);
                }
                idcu_strbuf_append(buf, "\n");
                break;
                
            case IDCU_METRIC_GAUGE:
                if (collector->namespace_prefix[0] != '\0') {
                    idcu_strbuf_append_format(buf, "# TYPE %s_%s gauge\n", 
                                              collector->namespace_prefix, m->name);
                    idcu_strbuf_append_format(buf, "%s_%s", collector->namespace_prefix, m->name);
                } else {
                    idcu_strbuf_append_format(buf, "# TYPE %s gauge\n", m->name);
                    idcu_strbuf_append(buf, m->name);
                }
                append_labels(buf, m->labels, m->label_count);
                idcu_strbuf_append_format(buf, " %llu", (unsigned long long)m->value);
                if (timestamp_ms > 0) {
                    idcu_strbuf_append_format(buf, " %llu", (unsigned long long)timestamp_ms);
                }
                idcu_strbuf_append(buf, "\n");
                break;
                
            case IDCU_METRIC_HISTOGRAM:
                if (collector->namespace_prefix[0] != '\0') {
                    idcu_strbuf_append_format(buf, "# TYPE %s_%s histogram\n", 
                                              collector->namespace_prefix, m->name);
                } else {
                    idcu_strbuf_append_format(buf, "# TYPE %s histogram\n", m->name);
                }
                
                if (m->bucket_count > 0) {
                    for (int b = 0; b < m->bucket_count; b++) {
                        if (collector->namespace_prefix[0] != '\0') {
                            idcu_strbuf_append_format(buf, "%s_%s_bucket", 
                                                      collector->namespace_prefix, m->name);
                        } else {
                            idcu_strbuf_append_format(buf, "%s_bucket", m->name);
                        }
                        
                        if (m->label_count > 0) {
                            idcu_strbuf_append(buf, "{");
                            for (int l = 0; l < m->label_count; l++) {
                                if (l > 0) idcu_strbuf_append(buf, ",");
                                idcu_strbuf_append_format(buf, "%s=\"%s\"", 
                                                          m->labels[l].key, m->labels[l].value);
                            }
                            idcu_strbuf_append_format(buf, ",le=\"%llu\"}", 
                                                      (unsigned long long)m->buckets[b]);
                        } else {
                            idcu_strbuf_append_format(buf, "{le=\"%llu\"}", 
                                                      (unsigned long long)m->buckets[b]);
                        }
                        
                        idcu_strbuf_append_format(buf, " %llu", 
                                                  (unsigned long long)m->bucket_counts[b]);
                        if (timestamp_ms > 0) {
                            idcu_strbuf_append_format(buf, " %llu", (unsigned long long)timestamp_ms);
                        }
                        idcu_strbuf_append(buf, "\n");
                    }
                }
                
                if (collector->namespace_prefix[0] != '\0') {
                    idcu_strbuf_append_format(buf, "%s_%s_sum", collector->namespace_prefix, m->name);
                } else {
                    idcu_strbuf_append_format(buf, "%s_sum", m->name);
                }
                append_labels(buf, m->labels, m->label_count);
                idcu_strbuf_append_format(buf, " %llu", (unsigned long long)m->sum);
                if (timestamp_ms > 0) {
                    idcu_strbuf_append_format(buf, " %llu", (unsigned long long)timestamp_ms);
                }
                idcu_strbuf_append(buf, "\n");
                
                if (collector->namespace_prefix[0] != '\0') {
                    idcu_strbuf_append_format(buf, "%s_%s_count", collector->namespace_prefix, m->name);
                } else {
                    idcu_strbuf_append_format(buf, "%s_count", m->name);
                }
                append_labels(buf, m->labels, m->label_count);
                idcu_strbuf_append_format(buf, " %llu", (unsigned long long)m->count);
                if (timestamp_ms > 0) {
                    idcu_strbuf_append_format(buf, " %llu", (unsigned long long)timestamp_ms);
                }
                idcu_strbuf_append(buf, "\n");
                break;
                
            case IDCU_METRIC_SUMMARY:
                if (collector->namespace_prefix[0] != '\0') {
                    idcu_strbuf_append_format(buf, "# TYPE %s_%s summary\n", 
                                              collector->namespace_prefix, m->name);
                    idcu_strbuf_append_format(buf, "%s_%s_sum", collector->namespace_prefix, m->name);
                } else {
                    idcu_strbuf_append_format(buf, "# TYPE %s summary\n", m->name);
                    idcu_strbuf_append_format(buf, "%s_sum", m->name);
                }
                append_labels(buf, m->labels, m->label_count);
                idcu_strbuf_append_format(buf, " %llu", (unsigned long long)m->sum);
                if (timestamp_ms > 0) {
                    idcu_strbuf_append_format(buf, " %llu", (unsigned long long)timestamp_ms);
                }
                idcu_strbuf_append(buf, "\n");
                
                if (collector->namespace_prefix[0] != '\0') {
                    idcu_strbuf_append_format(buf, "%s_%s_count", collector->namespace_prefix, m->name);
                } else {
                    idcu_strbuf_append_format(buf, "%s_count", m->name);
                }
                append_labels(buf, m->labels, m->label_count);
                idcu_strbuf_append_format(buf, " %llu", (unsigned long long)m->count);
                if (timestamp_ms > 0) {
                    idcu_strbuf_append_format(buf, " %llu", (unsigned long long)timestamp_ms);
                }
                idcu_strbuf_append(buf, "\n");
                break;
        }
    }

    idcu_mutex_unlock(&collector->lock);
    return IDCU_ERR_SUCCESS;
}

const char* idcu_metrics_get_grafana_dashboard_json(void)
{
    return g_grafana_dashboard_json;
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

int idcu_global_metrics_set_namespace(const char* ns)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_set_namespace(&g_global_collector, ns);
}

int idcu_global_metrics_register(const char* name, const char* desc, idcu_MetricType type)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_register(&g_global_collector, name, desc, type);
}

int idcu_global_metrics_register_with_labels(const char* name, const char* desc, 
                                               idcu_MetricType type,
                                               const idcu_MetricLabel* labels, int label_count)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_register_with_labels(&g_global_collector, name, desc, type, labels, label_count);
}

int idcu_global_metrics_set_histogram_buckets(const char* name,
                                                const uint64_t* buckets, int bucket_count)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_set_histogram_buckets(&g_global_collector, name, buckets, bucket_count);
}

int idcu_global_metrics_add_label(const char* name, const char* key, const char* value)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_add_label(&g_global_collector, name, key, value);
}

int idcu_global_metrics_inc(const char* name, uint64_t value)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_inc(&g_global_collector, name, value);
}

int idcu_global_metrics_dec(const char* name, uint64_t value)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_dec(&g_global_collector, name, value);
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

int idcu_global_metrics_export_prometheus_with_timestamp(idcu_StringBuf* buf, uint64_t timestamp_ms)
{
    if (!g_global_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return idcu_metrics_export_prometheus_with_timestamp(&g_global_collector, buf, timestamp_ms);
}
