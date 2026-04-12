#include <idcu/json/json.h>
#include <idcu/json/types.h>
#include <idcu/common/error_code.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
#include <stddef.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Need null-terminated string for JSON parsing
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        return 0;
    }
    memcpy(buffer, data, size);
    buffer[size] = '\0';
    
    // Test parse from string
    idcu_JsonValue* value = idcu_json_parse_string(buffer);
    if (value) {
        // Test serialization
        idcu_StringBuf sb;
        if (idcu_string_buf_init(&sb, 256) == IDCU_ERR_OK) {
            idcu_json_serialize(value, &sb, 0);
            idcu_string_buf_destroy(&sb);
        }
        
        // Test copy
        idcu_JsonValue* copy = idcu_json_clone(value);
        if (copy) {
            idcu_json_destroy(copy);
        }
        
        // Test type checking functions
        idcu_json_is_object(value);
        idcu_json_is_array(value);
        idcu_json_is_string(value);
        idcu_json_is_number(value);
        idcu_json_is_bool(value);
        idcu_json_is_null(value);
        
        idcu_json_destroy(value);
    }
    
    free(buffer);
    return 0;
}

#else

// Standalone test harness for non-fuzz builds
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <json-file>\n", argv[0]);
        return 1;
    }
    
    FILE* f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t* data = (uint8_t*)malloc(size);
    if (!data) {
        perror("malloc");
        fclose(f);
        return 1;
    }
    
    fread(data, 1, size, f);
    fclose(f);
    
    int ret = 0;
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    ret = LLVMFuzzerTestOneInput(data, size);
#else
    // Simple test without libFuzzer
    char* buffer = (char*)malloc(size + 1);
    if (buffer) {
        memcpy(buffer, data, size);
        buffer[size] = '\0';
        idcu_JsonValue* value = idcu_json_parse_string(buffer);
        if (value) {
            idcu_json_destroy(value);
        }
        free(buffer);
    }
#endif
    
    free(data);
    return ret;
}

#endif
