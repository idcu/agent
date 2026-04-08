#ifndef IDCU_MANAGEMENT_CLI_H
#define IDCU_MANAGEMENT_CLI_H

#include "idcu/common/error_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // 运行 CLI 命令
    int idcu_cli_run(int argc, char* argv[]);

    // 打印帮助信息
    void idcu_cli_print_help(void);

#ifdef __cplusplus
}
#endif

#endif  // IDCU_MANAGEMENT_CLI_H
