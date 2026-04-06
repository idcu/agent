#include <idcu/json/json.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const char* json_str = "{\"name\": \"test\", \"value\": 42}";
    idcu_JsonValue result;
    
    printf("Parsing JSON: %s\n", json_str);
    
    int ret = idcu_json_parse(json_str, &result);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Parse failed: %d\n", ret);
        return 1;
    }
    
    printf("Serializing back to string...\n");
    
    char* serialized = idcu_json_to_string_alloc(&result);
    if (serialized) {
        printf("Serialized: %s\n", serialized);
        free(serialized);
    } else {
        printf("Serialization failed!\n");
    }
    
    idcu_json_free(&result);
    printf("Done!\n");
    
    return 0;
}
