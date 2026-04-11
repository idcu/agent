#ifndef IDCU_TOOLS_MEMORY_DEBUGGER_H
#define IDCU_TOOLS_MEMORY_DEBUGGER_H

#include <idcu/common/config.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* address;
    size_t size;
    const char* file;
    int line;
    uint64_t timestamp;
    int in_use;
} idcu_MemoryAllocation;

int idcu_memory_debugger_init(void);
void idcu_memory_debugger_destroy(void);

void* idcu_debug_malloc(size_t size, const char* file, int line);
void* idcu_debug_calloc(size_t num, size_t size, const char* file, int line);
void* idcu_debug_realloc(void* ptr, size_t size, const char* file, int line);
void idcu_debug_free(void* ptr, const char* file, int line);

size_t idcu_memory_debugger_get_allocated_count(void);
size_t idcu_memory_debugger_get_total_allocated(void);
void idcu_memory_debugger_print_leaks(void);
void idcu_memory_debugger_reset(void);

#ifdef IDCU_DEBUG_MEMORY
#define idcu_malloc(size) idcu_debug_malloc(size, __FILE__, __LINE__)
#define idcu_calloc(num, size) idcu_debug_calloc(num, size, __FILE__, __LINE__)
#define idcu_realloc(ptr, size) idcu_debug_realloc(ptr, size, __FILE__, __LINE__)
#define idcu_free(ptr) idcu_debug_free(ptr, __FILE__, __LINE__)
#endif

#ifdef __cplusplus
}
#endif

#endif
