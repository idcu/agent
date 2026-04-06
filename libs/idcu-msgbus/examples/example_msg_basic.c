#include <idcu/msgbus/msg_bus.h>
#include <stdio.h>

int main(void) {
    printf("=== Basic Message Bus Example ===\n");
    
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    printf("Sending a message from module 1 to module 2...\n");
    
    idcu_StackContext ctx;
    idcu_ctx_init(&ctx, 1, 1001);
    int test_value = 42;
    memcpy(ctx.data, &test_value, sizeof(test_value));
    ctx.len = sizeof(test_value);
    
    int ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    if (ret != 0) {
        printf("Failed to send message: %d\n", ret);
        idcu_msg_bus_destroy(&bus);
        return 1;
    }
    
    printf("Message sent successfully, current queue count: %u\n", idcu_msg_get_count(&bus));
    
    printf("\nModule 2 receiving message...\n");
    
    idcu_Message msg;
    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != 0) {
        printf("Failed to receive message: %d\n", ret);
        idcu_msg_bus_destroy(&bus);
        return 1;
    }
    
    int received_value = 0;
    memcpy(&received_value, msg.data.data, sizeof(received_value));
    
    printf("Received message:\n");
    printf("  Source module: %u\n", msg.source_mod_id);
    printf("  Target module: %u\n", msg.target_mod_id);
    printf("  Priority: %d\n", msg.priority);
    printf("  Value: %d\n", received_value);
    printf("  Queue count after receive: %u\n", idcu_msg_get_count(&bus));
    
    idcu_msg_bus_destroy(&bus);
    
    printf("\n=== Example Complete ===\n");
    return 0;
}
