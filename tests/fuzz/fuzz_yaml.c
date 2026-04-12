#include <idcu/yaml/yaml.h>
#include <idcu/yaml/types.h>
#include <idcu/common/error_code.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
#include <stddef.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Need null-terminated string for YAML parsing
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        return 0;
    }
    memcpy(buffer, data, size);
    buffer[size] = '\0';
    
    // Test parse from string
    idcu_YamlNode* node = idcu_yaml_parse_string(buffer);
    if (node) {
        // Test serialization
        idcu_StringBuf sb;
        if (idcu_string_buf_init(&sb, 256) == IDCU_ERR_OK) {
            idcu_yaml_serialize(node, &sb);
            idcu_string_buf_destroy(&sb);
        }
        
        // Test clone
        idcu_YamlNode* copy = idcu_yaml_clone(node);
        if (copy) {
            idcu_yaml_destroy(copy);
        }
        
        // Test type checking functions
        idcu_yaml_is_map(node);
        idcu_yaml_is_sequence(node);
        idcu_yaml_is_scalar(node);
        
        idcu_yaml_destroy(node);
    }
    
    free(buffer);
    return 0;
}

#else

// Standalone test harness
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <yaml-file>\n", argv[0]);
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
    char* buffer = (char*)malloc(size + 1);
    if (buffer) {
        memcpy(buffer, data, size);
        buffer[size] = '\0';
        idcu_YamlNode* node = idcu_yaml_parse_string(buffer);
        if (node) {
            idcu_yaml_destroy(node);
        }
        free(buffer);
    }
    
    free(data);
    return ret;
}

#endif
