#include <idcu/testframework/testframework.h>
#include <idcu/msgbus/msgbus.h>
#include <string.h>

static int g_msg_received = 0;
static idcu_MsgTopic g_last_topic = 0;
static char g_last_data[256] = {0};
static size_t g_last_data_size = 0;
static void* g_last_user_data = NULL;

static void test_handler(idcu_MsgTopic topic,
                         const void* data,
                         size_t data_size,
                         void* user_data) {
    g_msg_received++;
    g_last_topic = topic;
    if (data && data_size > 0 && data_size < sizeof(g_last_data)) {
        memcpy(g_last_data, data, data_size);
        g_last_data[data_size] = '\0';
    }
    g_last_data_size = data_size;
    g_last_user_data = user_data;
}

TEST_CASE(msgbus_init_destroy) {
    idcu_MsgBus* bus = NULL;
    int ret = idcu_msgbus_init(&bus);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(bus != NULL);

    idcu_msgbus_destroy(bus);
    TEST_PASS();
}

TEST_CASE(msgbus_publish_subscribe) {
    idcu_MsgBus* bus = NULL;
    int ret = idcu_msgbus_init(&bus);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    g_msg_received = 0;
    idcu_MsgSubscriber* sub = NULL;
    int user_data = 42;

    ret = idcu_msgbus_subscribe(bus, 1, test_handler, &user_data, &sub);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    TEST_ASSERT(sub != NULL);

    const char* test_msg = "Hello, Message Bus!";
    ret = idcu_msgbus_publish(bus, 1, test_msg, strlen(test_msg), IDCU_MSG_PRIORITY_NORMAL);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_msgbus_process(bus);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    TEST_ASSERT(g_msg_received == 1);
    TEST_ASSERT(g_last_topic == 1);
    TEST_ASSERT(strcmp(g_last_data, test_msg) == 0);
    TEST_ASSERT(g_last_data_size == strlen(test_msg));
    TEST_ASSERT(g_last_user_data == &user_data);

    idcu_msgbus_unsubscribe(bus, sub);
    idcu_msgbus_destroy(bus);
    TEST_PASS();
}

TEST_CASE(msgbus_priority) {
    idcu_MsgBus* bus = NULL;
    int ret = idcu_msgbus_init(&bus);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    g_msg_received = 0;
    idcu_MsgSubscriber* sub = NULL;
    static int priorities_received[10] = {0};
    static int priority_index = 0;

    typedef struct {
        int priority;
        int index;
    } PriorityTestData;

    void priority_handler(idcu_MsgTopic topic,
                          const void* data,
                          size_t data_size,
                          void* user_data) {
        (void)topic;
        (void)user_data;
        if (data && data_size == sizeof(PriorityTestData)) {
            const PriorityTestData* pdata = (const PriorityTestData*)data;
            priorities_received[priority_index++] = pdata->priority;
        }
        g_msg_received++;
    }

    ret = idcu_msgbus_subscribe(bus, 2, priority_handler, NULL, &sub);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    PriorityTestData data1 = {IDCU_MSG_PRIORITY_LOW, 1};
    PriorityTestData data2 = {IDCU_MSG_PRIORITY_CRITICAL, 2};
    PriorityTestData data3 = {IDCU_MSG_PRIORITY_HIGH, 3};
    PriorityTestData data4 = {IDCU_MSG_PRIORITY_NORMAL, 4};

    idcu_msgbus_publish(bus, 2, &data1, sizeof(data1), IDCU_MSG_PRIORITY_LOW);
    idcu_msgbus_publish(bus, 2, &data2, sizeof(data2), IDCU_MSG_PRIORITY_CRITICAL);
    idcu_msgbus_publish(bus, 2, &data3, sizeof(data3), IDCU_MSG_PRIORITY_HIGH);
    idcu_msgbus_publish(bus, 2, &data4, sizeof(data4), IDCU_MSG_PRIORITY_NORMAL);

    idcu_msgbus_process(bus);

    TEST_ASSERT(g_msg_received == 4);
    TEST_ASSERT(priorities_received[0] == IDCU_MSG_PRIORITY_CRITICAL);
    TEST_ASSERT(priorities_received[1] == IDCU_MSG_PRIORITY_HIGH);
    TEST_ASSERT(priorities_received[2] == IDCU_MSG_PRIORITY_NORMAL);
    TEST_ASSERT(priorities_received[3] == IDCU_MSG_PRIORITY_LOW);

    idcu_msgbus_unsubscribe(bus, sub);
    idcu_msgbus_destroy(bus);
    TEST_PASS();
}

TEST_CASE(msgbus_error_cases) {
    idcu_MsgBus* bus = NULL;

    int ret = idcu_msgbus_init(NULL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_msgbus_init(&bus);
    TEST_ASSERT(ret == IDCU_ERR_OK);

    ret = idcu_msgbus_publish(NULL, 0, NULL, 0, IDCU_MSG_PRIORITY_NORMAL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_msgbus_publish(bus, IDCU_MSG_MAX_TOPICS, NULL, 0, IDCU_MSG_PRIORITY_NORMAL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    char big_data[IDCU_MSG_MAX_SIZE + 1] = {0};
    ret = idcu_msgbus_publish(bus, 0, big_data, IDCU_MSG_MAX_SIZE + 1, IDCU_MSG_PRIORITY_NORMAL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_msgbus_subscribe(NULL, 0, test_handler, NULL, NULL);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_MsgSubscriber* sub = NULL;
    ret = idcu_msgbus_subscribe(bus, 0, NULL, NULL, &sub);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    ret = idcu_msgbus_subscribe(bus, IDCU_MSG_MAX_TOPICS, test_handler, NULL, &sub);
    TEST_ASSERT(ret == IDCU_ERR_INVALID_ARG);

    idcu_msgbus_destroy(bus);
    TEST_PASS();
}

TEST_MAIN() {
    RUN_TEST(msgbus_init_destroy);
    RUN_TEST(msgbus_publish_subscribe);
    RUN_TEST(msgbus_priority);
    RUN_TEST(msgbus_error_cases);
    return 0;
}
