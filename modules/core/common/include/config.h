#ifndef IDCU_COMMON_CONFIG_H
#define IDCU_COMMON_CONFIG_H

#define IDCU_CONFIG_MAX_MODULES 32
#define IDCU_CONFIG_MAX_MSG 64
#define IDCU_CONFIG_STACK_CTX_SIZE 256
#define IDCU_CONFIG_LOG_LEVEL 1
#define IDCU_AGENT_VERSION_STR "1.0.0"
#define IDCU_CONFIG_FILE_PATH "config/agent.cfg"

// 权限定义
#define IDCU_PERM_SEND        (1U << 0)
#define IDCU_PERM_RECV        (1U << 1)
#define IDCU_PERM_RUN         (1U << 2)
#define IDCU_PERM_HW          (1U << 3)
#define IDCU_PERM_CONFIG      (1U << 4)
#define IDCU_PERM_LOG         (1U << 5)
#define IDCU_PERM_DEBUG       (1U << 6)
#define IDCU_PERM_MODULE_MGR  (1U << 7)

#endif // IDCU_COMMON_CONFIG_H