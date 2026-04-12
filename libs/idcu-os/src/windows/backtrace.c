#include "idcu/os/os.h"
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "dbghelp.lib")

static int g_backtrace_initialized = 0;

static void backtrace_init(void)
{
    if (g_backtrace_initialized) {
        return;
    }
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
    g_backtrace_initialized = 1;
}

int idcu_backtrace_capture(void** frames, size_t max_frames)
{
    backtrace_init();
    return CaptureStackBackTrace(0, (WORD)max_frames, frames, NULL);
}

int idcu_backtrace_format(void** frames, size_t num_frames, char* buf, size_t buf_len)
{
    if (!frames || num_frames == 0 || !buf || buf_len == 0) {
        return -1;
    }
    
    backtrace_init();
    
    HANDLE process = GetCurrentProcess();
    size_t offset = 0;
    
    for (size_t i = 0; i < num_frames && offset < buf_len - 1; i++) {
        DWORD64 address = (DWORD64)(frames[i]);
        char symbol_buf[sizeof(SYMBOL_INFO) + 256];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)symbol_buf;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 255;
        
        char line[256];
        if (SymFromAddr(process, address, 0, symbol)) {
            int written = snprintf(line, sizeof(line), "#%zu: %s\n", i, symbol->Name);
            if (written > 0 && offset + written < buf_len) {
                strcpy(buf + offset, line);
                offset += written;
            }
        } else {
            int written = snprintf(line, sizeof(line), "#%zu: 0x%p\n", i, frames[i]);
            if (written > 0 && offset + written < buf_len) {
                strcpy(buf + offset, line);
                offset += written;
            }
        }
    }
    
    return (int)offset;
}
