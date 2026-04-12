#include <idcu/config/config.h>
#include <idcu/config/types.h>
#include <idcu/yaml/yaml.h>
#include <idcu/json/json.h>
#include <idcu/common/error_code.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
#include <stddef.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) {
        return 0;
    }
    
    // Need null-terminated string
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        return 0;
    }
    memcpy(buffer, data, size);
    buffer[size] = '\0';
    
    // Test config parsing - try both YAML and JSON
    idcu_Config* config = idcu_config_create();
    if (config) {
        // Try to load from string
        // First try as YAML
        idcu_config_load_from_string(config, buffer);
        
        // Test getting values
        idcu_config_get_string(config, "test", "default");
        idcu_config_get_int(config, "test", 0);
        idcu_config_get_bool(config, "test", 0);
        idcu_config_get_double(config, "test", 0.0);
        
        idcu_config_destroy(config);
    }
    
    // Also test direct YAML parsing
    idcu_YamlNode* yaml = idcu_yaml_parse_string(buffer);
    if (yaml) {
        idcu_yaml_destroy(yaml);
    }
    
    // Also test direct JSON parsing
    idcu_JsonValue* json = idcu_json_parse_string(buffer);
    if (json) {
        idcu_json_destroy(json);
    }
    
    free(buffer);
    return 0;
}

#else

// Standalone test harness
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config-file>\n", argv[0]);
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
    
    char* buffer = (char*)malloc(size + 1);
    if (buffer) {
        memcpy(buffer, data, size);
        buffer[size] = '\0';
        
        idcu_Config* config = idcu_config_create();
        if (config) {
            idcu_config_load_from_string(config, buffer);
            idcu_config_destroy(config);
        }
        
        free(buffer);
    }
    
    free(data);
    return 0;
}

#endif
