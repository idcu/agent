#ifndef DEMO_IDCU_AGENT_H
#define DEMO_IDCU_AGENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int demo_idcu_agent_init(void);
    int demo_idcu_agent_run(void);
    int demo_idcu_agent_stop(void);

#ifdef __cplusplus
}
#endif

#endif
