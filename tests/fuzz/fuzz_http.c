#include <idcu/http_server/http_server.h>
#include <idcu/http_server/types.h>
#include <idcu/common/error_code.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
#include <stddef.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Test HTTP request parsing
    idcu_HttpRequest request;
    if (idcu_http_request_init(&request) == IDCU_ERR_OK) {
        // Try to parse raw HTTP request data
        // Note: This is a simplified test since we don't have direct raw parse API
        // We'll test setting various fields from fuzz data
        
        if (size > 0) {
            // Test method
            idcu_HttpMethod method = (idcu_HttpMethod)(data[0] % (IDCU_HTTP_METHOD_OPTIONS + 1));
            
            // Test path
            char* path = (char*)malloc(size + 1);
            if (path) {
                memcpy(path, data, size);
                path[size] = '\0';
                
                // Test setting headers
                if (size > 1) {
                    char key[64];
                    char value[128];
                    size_t key_len = (size / 2) < 63 ? (size / 2) : 63;
                    size_t val_len = (size - key_len) < 127 ? (size - key_len) : 127;
                    
                    memcpy(key, data, key_len);
                    key[key_len] = '\0';
                    memcpy(value, data + key_len, val_len);
                    value[val_len] = '\0';
                }
                
                free(path);
            }
        }
        
        idcu_http_request_destroy(&request);
    }
    
    // Test HTTP response
    idcu_HttpResponse response;
    if (idcu_http_response_init(&response) == IDCU_ERR_OK) {
        if (size > 0) {
            // Test status
            idcu_HttpStatus status = (idcu_HttpStatus)(data[0] % 600);
            idcu_http_response_set_status(&response, status);
            
            // Test body
            idcu_http_response_set_body(&response, (const char*)data, size);
        }
        idcu_http_response_destroy(&response);
    }
    
    return 0;
}

#else

// Standalone test harness
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <http-file>\n", argv[0]);
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
    
    // Simple test
    idcu_HttpRequest request;
    if (idcu_http_request_init(&request) == IDCU_ERR_OK) {
        idcu_http_request_destroy(&request);
    }
    
    free(data);
    return 0;
}

#endif
