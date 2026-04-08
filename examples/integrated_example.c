#include <stdio.h>
#include <string.h>
#include <idcu/log/log.h>
#include <idcu/cache/cache.h>
#include <idcu/msgbus/msg_bus.h>
#include <idcu/json/json.h>
#include <idcu/common/error_code.h>

#define TOPIC_METRICS "metrics"

static void on_message_received(const char* topic, const void* data, size_t size, void* user_data) {
    idcu_log_info("Received message on topic: %s", topic);
    if (size > 0 && data) {
        idcu_log_info("Message data: %.*s", (int)size, (const char*)data);
    }
}

int main(void) {
    printf("=== Integrated Example: Log + Cache + MsgBus + JSON ===\n\n");

    int ret;
    idcu_Cache* cache = NULL;
    idcu_MsgBus* msgbus = NULL;

    printf("1. Initialize Log System\n");
    printf("--------------------------\n");
    ret = idcu_log_init(IDCU_LOG_LEVEL_INFO, NULL);
    if (ret != IDCU_ERR_OK) {
        printf("Log init failed: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    idcu_log_info("Log system initialized");
    printf("\n");

    printf("2. Initialize Cache\n");
    printf("---------------------\n");
    ret = idcu_cache_init(&cache, 100, IDCU_CACHE_POLICY_LRU);
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("Cache init failed: %s", idcu_err_to_str(ret));
        idcu_log_shutdown();
        return 1;
    }
    idcu_log_info("Cache initialized");
    printf("\n");

    printf("3. Initialize Message Bus\n");
    printf("---------------------------\n");
    ret = idcu_msgbus_init(&msgbus);
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("MsgBus init failed: %s", idcu_err_to_str(ret));
        idcu_cache_destroy(cache);
        idcu_log_shutdown();
        return 1;
    }
    idcu_log_info("Message Bus initialized");
    printf("\n");

    printf("4. Subscribe to Topic\n");
    printf("-----------------------\n");
    ret = idcu_msgbus_subscribe(msgbus, TOPIC_METRICS, on_message_received, NULL);
    if (ret == IDCU_ERR_OK) {
        idcu_log_info("Subscribed to topic: %s", TOPIC_METRICS);
    }
    printf("\n");

    printf("5. Store Data in Cache\n");
    printf("------------------------\n");
    ret = idcu_cache_put_string(cache, "app_name", "IDCU Integrated Example", 0);
    if (ret == IDCU_ERR_OK) {
        idcu_log_info("Cached: app_name = IDCU Integrated Example");
    }

    ret = idcu_cache_put_int(cache, "version", 1, 0);
    if (ret == IDCU_ERR_OK) {
        idcu_log_info("Cached: version = 1");
    }
    printf("\n");

    printf("6. Retrieve from Cache and Publish\n");
    printf("------------------------------------\n");
    char app_name[256];
    ret = idcu_cache_get_string(cache, "app_name", app_name, sizeof(app_name));
    if (ret == IDCU_ERR_OK) {
        idcu_log_info("Retrieved from cache: app_name = %s", app_name);
        
        char message[512];
        snprintf(message, sizeof(message), "{\"app\":\"%s\",\"status\":\"running\"}", app_name);
        ret = idcu_msgbus_publish(msgbus, TOPIC_METRICS, message, strlen(message));
        if (ret == IDCU_ERR_OK) {
            idcu_log_info("Published message to %s", TOPIC_METRICS);
        }
    }
    printf("\n");

    printf("7. Process Messages\n");
    printf("---------------------\n");
    ret = idcu_msgbus_process(msgbus);
    if (ret == IDCU_ERR_OK) {
        idcu_log_info("Message bus processed");
    }
    printf("\n");

    printf("8. Cleanup\n");
    printf("------------\n");
    idcu_msgbus_destroy(msgbus);
    idcu_log_info("Message Bus destroyed");

    idcu_cache_destroy(cache);
    idcu_log_info("Cache destroyed");

    idcu_log_shutdown();
    printf("Log system shutdown\n\n");

    printf("=== Integrated Example Complete ===\n");
    return 0;
}
