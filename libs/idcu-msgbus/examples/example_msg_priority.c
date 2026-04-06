#include <idcu/msgbus/msg_bus.h>
#include <stdio.h>

int main(void) {
    printf("=== Message Priority Example ===\n");
    printf("Messages should be received in order of priority: Realtime -> High -> Normal -> Low\n\n");
    
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    idcu_StackContext ctx_low, ctx_norm, ctx_high, ctx_realtime;
    int val_low = 1, val_norm = 2, val_high = 3, val_realtime = 4;
    
    idcu_ctx_init(&ctx_low, 1, 2001);
    memcpy(ctx_low.data, &val_low, sizeof(val_low));
    ctx_low.len = sizeof(val_low);
    
    idcu_ctx_init(&ctx_norm, 1, 2002);
    memcpy(ctx_norm.data, &val_norm, sizeof(val_norm));
    ctx_norm.len = sizeof(val_norm);
    
    idcu_ctx_init(&ctx_high, 1, 2003);
    memcpy(ctx_high.data, &val_high, sizeof(val_high));
    ctx_high.len = sizeof(val_high);
    
    idcu_ctx_init(&ctx_realtime, 1, 2004);
    memcpy(ctx_realtime.data, &val_realtime, sizeof(val_realtime));
    ctx_realtime.len = sizeof(val_realtime);
    
    printf("Sending messages in order: Low -> Normal -> High -> Realtime\n");
    
    idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_LOW, &ctx_low);
    idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx_norm);
    idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_HIGH, &ctx_high);
    idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_REALTIME, &ctx_realtime);
    
    printf("Total messages in queue: %u\n\n", idcu_msg_get_count(&bus));
    
    printf("Receiving messages (should be in priority order):\n");
    
    const char* priority_names[] = {"Low", "Normal", "High", "Realtime"};
    
    for (int i = 0; i < 4; i++) {
        idcu_Message msg;
        int ret = idcu_msg_recv(&bus, 2, &msg);
        if (ret == 0) {
            int val = 0;
            memcpy(&val, msg.data.data, sizeof(val));
            printf("  Received: value=%d, priority=%s\n", val, priority_names[msg.priority]);
        }
    }
    
    printf("\nQueue is now empty: %u\n", idcu_msg_get_count(&bus));
    
    idcu_msg_bus_destroy(&bus);
    
    printf("\n=== Example Complete ===\n");
    return 0;
}
