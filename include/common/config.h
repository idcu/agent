#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_MAX_MODULES 32
#define CONFIG_MAX_MSG 64
#define CONFIG_STACK_CTX_SIZE 256
#define CONFIG_LOG_LEVEL 1
#define AGENT_VERSION_STR "1.0.0"
#define CONFIG_FILE_PATH "config/agent.cfg"

// 权限定义
#define PERM_SEND        (1U << 0)
#define PERM_RECV        (1U << 1)
#define PERM_RUN         (1U << 2)
#define PERM_HW          (1U << 3)
#define PERM_CONFIG      (1U << 4)
#define PERM_LOG         (1U << 5)
#define PERM_DEBUG       (1U << 6)
#define PERM_MODULE_MGR  (1U << 7)

#endif