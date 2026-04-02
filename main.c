#include "core/include/core.h"

int main() {
    printf("=== 全模块化 AI 引擎（Core + Base + Biz）===\n");
    core_init();
    core_load_modules("modules.conf");
    core_run_agent("scan -> read -> edit");
    printf("=== 运行结束 ===\n");
    return 0;
}
