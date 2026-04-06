#include <idcu/json/json.h>
#include <stdio.h>

int main(void)
{
    const char* json_str = "{\"name\": \"test\", \"value\": 42, \"enabled\": true, \"items\": [1, 2, 3]}";
    idcu_JsonValue result;
    
    printf("Parsing JSON: %s\n", json_str);
    
    int ret = idcu_json_parse(json_str, &result);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Parse failed: %d\n", ret);
        return 1;
    }
    
    printf("Parse successful!\n");
    
    idcu_JsonValue* name_val = idcu_json_object_get(result.data.object_val, "name");
    if (name_val) {
        const char* str;
        idcu_json_get_string(name_val, &str);
        printf("name: %s\n", str);
    }
    
    idcu_JsonValue* value_val = idcu_json_object_get(result.data.object_val, "value");
    if (value_val) {
        int64_t val;
        idcu_json_get_int(value_val, &val);
        printf("value: %lld\n", (long long)val);
    }
    
    idcu_JsonValue* enabled_val = idcu_json_object_get(result.data.object_val, "enabled");
    if (enabled_val) {
        int val;
        idcu_json_get_bool(enabled_val, &val);
        printf("enabled: %s\n", val ? "true" : "false");
    }
    
    idcu_JsonValue* items_val = idcu_json_object_get(result.data.object_val, "items");
    if (items_val && items_val->type == IDCU_JSON_TYPE_ARRAY) {
        printf("items: ");
        for (size_t i = 0; i < idcu_json_array_size(items_val->data.array_val); i++) {
            idcu_JsonValue* item = idcu_json_array_get(items_val->data.array_val, i);
            int64_t val;
            idcu_json_get_int(item, &val);
            printf("%lld ", (long long)val);
        }
        printf("\n");
    }
    
    idcu_json_free(&result);
    printf("Done!\n");
    
    return 0;
}
