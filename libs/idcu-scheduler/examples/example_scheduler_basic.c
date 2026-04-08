#include <idcu/common/error_code.h>
#include <idcu/scheduler/scheduler.h>
#include <stdio.h>
#include <unistd.h>

static int task1_count = 0;
static int task2_count = 0;

static void task1_callback(void *user_data) {
    task1_count++;
    printf("[TASK 1] Executed %d times\n", task1_count);
}

static void task2_callback(void *user_data) {
    task2_count++;
    printf("[TASK 2] Executed %d times\n", task2_count);
}

static void print_task_status(const char *prefix, const idcu_TaskInfo *task) {
    const char *status_str = "UNKNOWN";
    switch (task->status) {
    case IDCU_TASK_STATUS_IDLE:
        status_str = "IDLE";
        break;
    case IDCU_TASK_STATUS_RUNNING:
        status_str = "RUNNING";
        break;
    case IDCU_TASK_STATUS_COMPLETED:
        status_str = "COMPLETED";
        break;
    case IDCU_TASK_STATUS_FAILED:
        status_str = "FAILED";
        break;
    case IDCU_TASK_STATUS_PAUSED:
        status_str = "PAUSED";
        break;
    }
    printf("%s: %s\n", prefix, task->name);
    printf("  Status: %s\n", status_str);
    printf("  Run count: %llu\n", (unsigned long long)task->run_count);
    printf("  Success: %llu\n", (unsigned long long)task->success_count);
    printf("  Fail: %llu\n", (unsigned long long)task->fail_count);
}

int main(void) {
    printf("=== idcu-scheduler Basic Example ===\n\n");

    int ret;
    idcu_Scheduler scheduler;

    printf("1. Initialize Scheduler\n");
    printf("-------------------------\n");
    ret = idcu_scheduler_init(&scheduler);
    if (ret != IDCU_ERR_OK) {
        printf("Scheduler init failed: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("Scheduler initialized\n\n");

    printf("2. Add Tasks\n");
    printf("--------------\n");

    uint64_t task1_id = idcu_scheduler_add_task(&scheduler, "onetime_task", task1_callback, NULL);
    printf("Added one-time task, ID: %llu\n", (unsigned long long)task1_id);

    uint64_t task2_id = idcu_scheduler_add_interval_task(&scheduler, "interval_task", 1000, 1,
                                                         task2_callback, NULL);
    printf("Added interval task (1000ms), ID: %llu\n", (unsigned long long)task2_id);
    printf("\n");

    printf("3. List All Tasks\n");
    printf("-------------------\n");
    idcu_TaskInfo tasks[IDCU_TASK_MAX];
    int task_count = IDCU_TASK_MAX;
    ret = idcu_scheduler_list_tasks(&scheduler, tasks, &task_count);
    if (ret == IDCU_ERR_OK) {
        printf("Found %d tasks:\n", task_count);
        for (int i = 0; i < task_count; i++) {
            printf("  - %s (ID: %llu)\n", tasks[i].name, (unsigned long long)tasks[i].task_id);
        }
    }
    printf("\n");

    printf("4. Get Task Info\n");
    printf("------------------\n");
    idcu_TaskInfo task_info;
    ret = idcu_scheduler_get_task(&scheduler, task1_id, &task_info);
    if (ret == IDCU_ERR_OK) {
        print_task_status("Task 1 Info", &task_info);
    }
    printf("\n");

    printf("5. Run Task Immediately\n");
    printf("-------------------------\n");
    ret = idcu_scheduler_run_task_now(&scheduler, task1_id);
    if (ret == IDCU_ERR_OK) {
        printf("Task 1 executed immediately\n");
    }
    printf("\n");

    printf("6. Pause and Resume Task\n");
    printf("--------------------------\n");
    ret = idcu_scheduler_pause_task(&scheduler, task2_id);
    if (ret == IDCU_ERR_OK) {
        printf("Task 2 paused\n");
    }

    ret = idcu_scheduler_resume_task(&scheduler, task2_id);
    if (ret == IDCU_ERR_OK) {
        printf("Task 2 resumed\n");
    }
    printf("\n");

    printf("7. Remove Task\n");
    printf("----------------\n");
    ret = idcu_scheduler_remove_task(&scheduler, task1_id);
    if (ret == IDCU_ERR_OK) {
        printf("Task 1 removed\n");
    }
    printf("\n");

    printf("8. Destroy Scheduler\n");
    printf("----------------------\n");
    idcu_scheduler_destroy(&scheduler);
    printf("Scheduler destroyed\n\n");

    printf("=== Example Complete ===\n");
    return 0;
}
