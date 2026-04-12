#include <idcu/msgbus/msgbus.h>
#include <idcu/msgbus/types.h>
#include <idcu/common/error_code.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
#include <stddef.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Initialize message bus
    idcu_MsgBus bus;
    if (idcu_msgbus_init(&bus) != IDCU_SUCCESS) {
        return 0;
    }
    
    // Test topic creation from fuzz data
    if (size > 0 && size <= 256) {
        char topic[257];
        memcpy(topic, data, size);
        topic[size] = '\0';
        
        // Create topic
        idcu_msgbus_create_topic(&bus, topic);
        
        // Test message publishing
        size_t payload_size = size / 2;
        if (payload_size > 0) {
            idcu_Message* msg = idcu_message_create(topic, data, payload_size);
            if (msg) {
                idcu_msgbus_publish(&bus, msg);
                idcu_message_destroy(msg);
            }
        }
    }
    
    // Test with empty/small data
    if (size == 0) {
        idcu_msgbus_create_topic(&bus, "");
    }
    
    idcu_msgbus_destroy(&bus);
    return 0;
}

#else

// Standalone test harness
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <data-file>\n", argv[0]);
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
    idcu_MsgBus bus;
    if (idcu_msgbus_init(&bus) == IDCU_SUCCESS) {
        char topic[257];
        size_t topic_size = size < 256 ? size : 256;
        memcpy(topic, data, topic_size);
        topic[topic_size] = '\0';
        idcu_msgbus_create_topic(&bus, topic);
        idcu_msgbus_destroy(&bus);
    }
    
    free(data);
    return 0;
}

#endif
