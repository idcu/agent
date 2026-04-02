#ifndef MODULE_DEF_H
#define MODULE_DEF_H

typedef struct {
    const char *name;
    int (*init)(void);
    int (*run)(void);
    int (*stop)(void);
} ModuleInterface;

#define REGISTER_MODULE(name, init_fn, run_fn, stop_fn) \
    const ModuleInterface __module_##name \
        __attribute__((section(".modules"))) = { \
            #name, init_fn, run_fn, stop_fn \
        }

#endif