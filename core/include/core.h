#ifndef CORE_H
#define CORE_H

void core_init(void);
int core_load_modules(const char *conf_path);
void core_run_agent(const char *task);

#endif
