/**
 * @file example_error_recovery.c
 * @brief IDCU 错误恢复机制示例
 *
 * 本示例展示了如何使用 idcu-common 库的错误处理系统
 * 实现健壮的错误恢复机制。
 */

#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/hash_map.h>
#include <idcu/log/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 示例：带有重试机制的函数
 *
 * 这个函数模拟了一个可能失败的操作，
 * 并展示了如何实现自动重试逻辑。
 */
static int operation_with_retry(int max_retries, int* result)
{
    int retries = 0;
    int err;

    while (retries < max_retries) {
        /* 模拟一个可能失败的操作 */
        err = (rand() % 3 == 0) ? IDCU_ERR_OK : IDCU_ERR_GENERAL;

        if (err == IDCU_ERR_OK) {
            *result = 42; /* 模拟成功结果 */
            idcu_log_info("操作成功，重试次数: %d", retries);
            return IDCU_ERR_OK;
        }

        retries++;
        idcu_log_warn("操作失败，正在重试 (%d/%d)", retries, max_retries);

        /* 指数退避等待 */
        if (retries < max_retries) {
            /* 在实际代码中，这里应该使用适当的延迟机制 */
        }
    }

    idcu_log_error("操作在 %d 次重试后仍然失败", max_retries);
    IDCU_ERR_SET(IDCU_ERR_GENERAL, "操作重试次数超限");
    return IDCU_ERR_GENERAL;
}

/**
 * @brief 示例：资源清理函数
 *
 * 这个函数展示了如何确保资源被正确清理，
 * 即使在发生错误的情况下。
 */
static int resource_handling_example(void)
{
    idcu_Vector vec;
    idcu_HashMap map;
    int err;
    int* data = NULL;

    /* 初始化资源 */
    err = idcu_vector_init(&vec, sizeof(int));
    if (err != IDCU_ERR_OK) {
        idcu_log_error("初始化向量失败");
        return err;
    }

    err = idcu_hash_map_init(&map, 0, sizeof(int));
    if (err != IDCU_ERR_OK) {
        idcu_log_error("初始化哈希表失败");
        idcu_vector_destroy(&vec);
        return err;
    }

    /* 分配临时内存 */
    data = (int*)malloc(sizeof(int) * 100);
    if (data == NULL) {
        idcu_log_error("分配内存失败");
        idcu_hash_map_destroy(&map);
        idcu_vector_destroy(&vec);
        IDCU_ERR_SET(IDCU_ERR_NO_MEMORY, "内存分配失败");
        return IDCU_ERR_NO_MEMORY;
    }

    /* 使用资源进行一些操作 */
    idcu_log_info("资源初始化成功，开始操作...");

    /* 模拟操作可能失败 */
    if (rand() % 2 == 0) {
        idcu_log_error("模拟操作失败");
        err = IDCU_ERR_GENERAL;
        IDCU_ERR_SET(err, "模拟操作失败");
        goto cleanup;
    }

    idcu_log_info("所有操作成功完成");
    err = IDCU_ERR_OK;

cleanup:
    /* 确保所有资源都被清理 */
    if (data != NULL) {
        free(data);
    }
    idcu_hash_map_destroy(&map);
    idcu_vector_destroy(&vec);

    idcu_log_info("资源清理完成");
    return err;
}

/**
 * @brief 示例：使用错误链
 *
 * 这个函数展示了如何使用错误链来传播
 * 详细的错误信息。
 */
static int error_chain_example(void)
{
    int err;
    idcu_ErrorInfo* cause = NULL;

    /* 模拟第一层错误 */
    idcu_log_info("开始执行多层操作...");

    err = IDCU_ERR_INVALID_PARAM;
    IDCU_ERR_SET(err, "第一层：无效参数");

    /* 获取第一层错误作为cause */
    cause = idcu_err_clone_error(idcu_err_get_last_error());

    /* 模拟第二层错误，附加错误链 */
    err = IDCU_ERR_MODULE_INIT;
    IDCU_ERR_SET_WITH_CAUSE(err, "第二层：模块初始化失败", cause);

    /* 获取并打印完整的错误链 */
    const idcu_ErrorInfo* final_error = idcu_err_get_last_error();
    if (final_error != NULL) {
        char buffer[1024];
        idcu_err_format_error_chain(final_error, buffer, sizeof(buffer));
        idcu_log_error("完整错误信息:\n%s", buffer);
    }

    /* 清理 */
    if (cause != NULL) {
        idcu_err_free_error(cause);
    }

    return IDCU_ERR_OK;
}

/**
 * @brief 示例：带状态恢复的有限状态机
 *
 * 这个函数展示了如何实现一个能够从错误状态
 * 恢复的简单状态机。
 */
typedef enum {
    STATE_INIT,
    STATE_READY,
    STATE_PROCESSING,
    STATE_ERROR,
    STATE_DONE
} StateMachineState;

static int state_machine_with_recovery(void)
{
    StateMachineState state = STATE_INIT;
    int err = IDCU_ERR_OK;
    int recovery_attempts = 0;
    const int MAX_RECOVERY_ATTEMPTS = 3;

    idcu_log_info("启动带恢复功能的状态机...");

    while (state != STATE_DONE) {
        switch (state) {
            case STATE_INIT:
                idcu_log_info("状态：初始化");
                state = STATE_READY;
                break;

            case STATE_READY:
                idcu_log_info("状态：就绪");
                state = STATE_PROCESSING;
                break;

            case STATE_PROCESSING:
                idcu_log_info("状态：处理中");
                /* 模拟可能失败的处理 */
                if (rand() % 3 == 0) {
                    idcu_log_error("处理失败");
                    err = IDCU_ERR_GENERAL;
                    state = STATE_ERROR;
                } else {
                    idcu_log_info("处理成功");
                    state = STATE_DONE;
                }
                break;

            case STATE_ERROR:
                idcu_log_warn("状态：错误，尝试恢复...");
                if (recovery_attempts < MAX_RECOVERY_ATTEMPTS) {
                    recovery_attempts++;
                    idcu_log_info("恢复尝试 %d/%d", recovery_attempts, MAX_RECOVERY_ATTEMPTS);
                    /* 这里可以执行特定的恢复操作 */
                    state = STATE_READY;
                    err = IDCU_ERR_OK;
                } else {
                    idcu_log_error("恢复尝试次数超限，放弃");
                    IDCU_ERR_SET(IDCU_ERR_GENERAL, "状态机恢复失败");
                    return IDCU_ERR_GENERAL;
                }
                break;

            case STATE_DONE:
                idcu_log_info("状态：完成");
                break;
        }
    }

    idcu_log_info("状态机成功完成");
    return IDCU_ERR_OK;
}

int main(void)
{
    int result;
    int err;

    /* 初始化日志系统 */
    idcu_log_init(IDCU_LOG_LEVEL_INFO, IDCU_LOG_OUTPUT_CONSOLE);

    idcu_log_info("=== IDCU 错误恢复机制示例 ===\n");

    /* 示例1: 带有重试机制的操作 */
    idcu_log_info("\n--- 示例1: 重试机制 ---");
    err = operation_with_retry(3, &result);
    if (err == IDCU_ERR_OK) {
        idcu_log_info("结果: %d", result);
    }

    /* 示例2: 资源清理 */
    idcu_log_info("\n--- 示例2: 资源清理 ---");
    resource_handling_example();

    /* 示例3: 错误链 */
    idcu_log_info("\n--- 示例3: 错误链 ---");
    idcu_err_clear_last_error();
    error_chain_example();

    /* 示例4: 状态机恢复 */
    idcu_log_info("\n--- 示例4: 状态机恢复 ---");
    state_machine_with_recovery();

    idcu_log_info("\n=== 示例程序结束 ===");

    idcu_log_destroy();
    return 0;
}
