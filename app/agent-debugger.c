#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "micro_kernel.h"
#include "module_def.h"
#include "module_registry.h"

static idcu_MicroKernel g_kernel;
static volatile int g_should_exit = 0;
static volatile int g_debug_mode = 1;

static const char *module_state_to_str(idcu_ModuleState state) {
    switch (state) {
    case IDCU_MOD_STATE_UNINIT:
        return "UNINIT";
    case IDCU_MOD_STATE_INITED:
        return "INITED";
    case IDCU_MOD_STATE_RUNNING:
        return "RUNNING";
    case IDCU_MOD_STATE_STOPPED:
        return "STOPPED";
    case IDCU_MOD_STATE_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

static void print_module_status(idcu_MicroKernel *k) {
    printf("\n========================================\n");
    printf("         MODULE STATUS\n");
    printf("========================================\n");
    printf("Total tracked modules: %u\n", k->tracked_cnt);
    printf("\n%-20s %-10s %-10s\n", "NAME", "STATE", "DYNAMIC");
    printf("----------------------------------------\n");

    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tm = &k->tracked_modules[i];
        if (tm->iface) {
            printf("%-20s %-10s %-10s\n", tm->iface->name, module_state_to_str(tm->state),
                   tm->is_dynamic ? "YES" : "NO");
        }
    }
    printf("========================================\n\n");
}

static void print_message_bus_stats(idcu_MicroKernel *k) {
    printf("\n========================================\n");
    printf("      MESSAGE BUS STATISTICS\n");
    printf("========================================\n");

    uint32_t total_count = idcu_msg_get_count(&k->msg);
    printf("Total messages in bus: %u\n", total_count);

    printf("\nPriority queue status:\n");
    const char *prio_names[] = {"LOW", "NORMAL", "HIGH", "REALTIME"};
    for (int i = 0; i < IDCU_MSG_PRIO_COUNT; i++) {
        idcu_PriorityQueue *q = &k->msg.prio_queues[i];
        uint32_t count =
            (q->tail >= q->head) ? (q->tail - q->head) : (IDCU_MSG_QUEUE_SIZE - q->head + q->tail);
        printf("  %-10s: %u messages\n", prio_names[i], count);
    }

    printf("========================================\n\n");
}

static void print_resource_usage(void) {
    printf("\n========================================\n");
    printf("      RESOURCE USAGE\n");
    printf("========================================\n");

#ifdef _WIN32
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);
    printf("Total physical memory: %llu MB\n", memStatus.ullTotalPhys / (1024 * 1024));
    printf("Available physical memory: %llu MB\n", memStatus.ullAvailPhys / (1024 * 1024));
    printf("Memory load: %lu%%\n", memStatus.dwMemoryLoad);
#else
    printf("Resource usage info (Linux/macOS)\n");
    printf("Note: Detailed resource monitoring requires system-specific APIs\n");
#endif

    printf("========================================\n\n");
}

static void print_help(void) {
    printf("\n========================================\n");
    printf("         DEBUGGER COMMANDS\n");
    printf("========================================\n");
    printf("  help     - Show this help message\n");
    printf("  status   - Show module status\n");
    printf("  msgbus   - Show message bus statistics\n");
    printf("  resource - Show resource usage\n");
    printf("  all      - Show all debug info\n");
    printf("  loglevel <level> - Set log level (debug/info/warn/error)\n");
    printf("  quit     - Exit debugger and stop agent\n");
    printf("========================================\n\n");
}

static void set_log_level(const char *level_str) {
    idcu_LogLevel level;

    if (strcmp(level_str, "debug") == 0) {
        level = IDCU_LOG_DEBUG;
    } else if (strcmp(level_str, "info") == 0) {
        level = IDCU_LOG_INFO;
    } else if (strcmp(level_str, "warn") == 0 || strcmp(level_str, "warning") == 0) {
        level = IDCU_LOG_WARN;
    } else if (strcmp(level_str, "error") == 0) {
        level = IDCU_LOG_ERROR;
    } else {
        printf("Invalid log level: %s\n", level_str);
        printf("Valid levels: debug, info, warn, error\n");
        return;
    }

    idcu_log_set_level(level);
    printf("Log level set to: %s\n", level_str);
}

static void process_command(const char *cmd) {
    char cmd_copy[256];
    strncpy(cmd_copy, cmd, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    char *token = strtok(cmd_copy, " \t\n");
    if (!token)
        return;

    if (strcmp(token, "help") == 0) {
        print_help();
    } else if (strcmp(token, "status") == 0) {
        print_module_status(&g_kernel);
    } else if (strcmp(token, "msgbus") == 0) {
        print_message_bus_stats(&g_kernel);
    } else if (strcmp(token, "resource") == 0) {
        print_resource_usage();
    } else if (strcmp(token, "all") == 0) {
        print_module_status(&g_kernel);
        print_message_bus_stats(&g_kernel);
        print_resource_usage();
    } else if (strcmp(token, "loglevel") == 0) {
        char *level = strtok(NULL, " \t\n");
        if (level) {
            set_log_level(level);
        } else {
            printf("Usage: loglevel <debug|info|warn|error>\n");
        }
    } else if (strcmp(token, "quit") == 0 || strcmp(token, "exit") == 0) {
        g_should_exit = 1;
        g_kernel.should_exit = 1;
    } else {
        printf("Unknown command: %s\n", token);
        printf("Type 'help' for available commands\n");
    }
}

#ifdef _WIN32
#include <conio.h>
#include <windows.h>

static BOOL WINAPI win_ctrl_handler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        printf("\nReceived Ctrl+C, shutting down...\n");
        g_should_exit = 1;
        g_kernel.should_exit = 1;
        return TRUE;
    }
    return FALSE;
}

static void check_input(void) {
    if (_kbhit()) {
        char cmd[256];
        int i = 0;
        while (_kbhit() && i < sizeof(cmd) - 1) {
            int ch = _getch();
            if (ch == '\r') {
                cmd[i] = '\0';
                break;
            } else if (ch == '\b') {
                if (i > 0) {
                    i--;
                    printf("\b \b");
                }
            } else {
                cmd[i++] = (char)ch;
                printf("%c", ch);
            }
        }
        cmd[i] = '\0';
        printf("\n");
        process_command(cmd);
        printf("> ");
        fflush(stdout);
    }
}
#else
#include <poll.h>
#include <termios.h>
#include <unistd.h>

static struct termios g_orig_termios;

static void unix_sig_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\nReceived signal %d, shutting down...\n", sig);
        g_should_exit = 1;
        g_kernel.should_exit = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
    }
}

static void check_input(void) {
    struct pollfd fds;
    fds.fd = STDIN_FILENO;
    fds.events = POLLIN;

    int ret = poll(&fds, 1, 100);
    if (ret > 0) {
        char cmd[256];
        if (fgets(cmd, sizeof(cmd), stdin)) {
            process_command(cmd);
            printf("> ");
            fflush(stdout);
        }
    }
}
#endif

int main(void) {
    printf("========================================\n");
    printf("   IDCU Agent Debugger\n");
    printf("========================================\n");
    printf("Type 'help' for available commands\n\n");

    int log_ret = idcu_log_init("logs/agent-debug.log", IDCU_LOG_DEBUG);
    if (log_ret != IDCU_ERR_SUCCESS) {
        fprintf(stderr, "Warning: Failed to initialize log system\n");
        idcu_log_set_level(IDCU_LOG_DEBUG);
    }

    idcu_log_set_rotate_policy(IDCU_LOG_ROTATE_SIZE, 10 * 1024 * 1024, 0, 5);

    idcu_kernel_init(&g_kernel);
    g_kernel.should_exit = 0;

#ifdef _WIN32
    SetConsoleCtrlHandler(win_ctrl_handler, TRUE);
#else
    tcgetattr(STDIN_FILENO, &g_orig_termios);
    struct termios new_termios = g_orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    signal(SIGINT, unix_sig_handler);
    signal(SIGTERM, unix_sig_handler);
#endif

    idcu_kernel_start_modules(&g_kernel);

    printf("> ");
    fflush(stdout);

    while (!g_should_exit) {
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
        check_input();

        if (!g_kernel.should_exit) {
            idcu_kernel_run(&g_kernel);
        }
    }

    printf("\nShutting down...\n");
    idcu_kernel_stop(&g_kernel);
    idcu_log_shutdown();

#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
#endif

    printf("Agent debugger stopped.\n");
    return 0;
}
