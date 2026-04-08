#ifndef DEMO_CUSTOM_H
#define DEMO_CUSTOM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        char     name[64];
        int32_t  value;
        uint64_t timestamp;
    } CustomData;

    int custom_init(void);
    int custom_process(CustomData* data);
    int custom_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
