#include "idcu/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

static int config_changed_count = 0;
static int should_stop = 0;

void config_change_callback(const char *section, const char *key, const char *old_value,
                            const char *new_value, void *user_data) {
    config_changed_count++;
    printf("\n[CONFIG CHANGE DETECTED]\n");
    printf("  Section: %s\n", section);
    printf("  Key:     %s\n", key);
    printf("  Old:     %s\n", old_value ? old_value : "(null)");
    printf("  New:     %s\n", new_value ? new_value : "(null)");
    printf("  Total changes: %d\n", config_changed_count);
}

void print_current_config(void) {
    printf("\n[CURRENT CONFIGURATION]\n");
    printf("  [app]\n");
    printf("    name:    %s\n", idcu_config_get_string("app", "name", "unknown"));
    printf("    version: %d\n", idcu_config_get_int("app", "version", 0));
    printf("    debug:   %s\n", idcu_config_get_bool("app", "debug", 0) ? "true" : "false");
    printf("    timeout: %.1f\n", idcu_config_get_double("app", "timeout", 0.0));
    printf("  [server]\n");
    printf("    host:    %s\n", idcu_config_get_string("server", "host", "localhost"));
    printf("    port:    %d\n", idcu_config_get_int("server", "port", 8080));
    printf("  [database]\n");
    printf("    url:     %s\n", idcu_config_get_string("database", "url", "sqlite://:memory:"));
    printf("    max_conn:%d\n", idcu_config_get_int("database", "max_connections", 10));
    printf("  [logging]\n");
    printf("    level:   %s\n", idcu_config_get_string("logging", "level", "info"));
    printf("    file:    %s\n", idcu_config_get_string("logging", "file", "app.log"));
}

int main(int argc, char *argv[]) {
    const char *config_file = (argc > 1) ? argv[1] : "example_config.cfg";

    printf("idcu-config Hot Reload Example\n");
    printf("================================\n\n");

    printf("Configuration file: %s\n", config_file);
    printf("\nPress Ctrl+C to stop the example\n");
    printf("While running, edit the config file to see hot reload in action!\n\n");

    int ret = idcu_config_init(config_file);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to initialize config: %d\n", ret);
        printf("Creating a default configuration file...\n");

        ret = idcu_config_init(NULL);
        if (ret != IDCU_ERR_SUCCESS) {
            printf("Failed to initialize empty config: %d\n", ret);
            return 1;
        }

        idcu_config_set_string("app", "name", "IDCU Example App");
        idcu_config_set_int("app", "version", 1);
        idcu_config_set_bool("app", "debug", 1);
        idcu_config_set_double("app", "timeout", 30.0);

        idcu_config_set_string("server", "host", "localhost");
        idcu_config_set_int("server", "port", 8080);

        idcu_config_set_string("database", "url", "sqlite://data.db");
        idcu_config_set_int("database", "max_connections", 20);

        idcu_config_set_string("logging", "level", "info");
        idcu_config_set_string("logging", "file", "app.log");

        idcu_config_save(config_file);
        printf("Created default config file: %s\n", config_file);
    }

    idcu_config_register_change_callback(config_change_callback, NULL);

    ret = idcu_config_watch_start();
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Warning: Failed to start config watcher: %d\n", ret);
        printf("Continuing without auto-reload. Manual reload still available.\n");
    } else {
        printf("Config watcher started successfully!\n");
    }

    print_current_config();

    printf("\n[RUNNING] Monitoring config changes...\n");
    printf("  Commands:\n");
    printf("    r - Reload config manually\n");
    printf("    s - Save current config\n");
    printf("    p - Print current config\n");
    printf("    q - Quit\n\n");

    char input[256];
    int iteration = 0;

    while (!should_stop) {
        iteration++;

        printf("\r[%d] Waiting... (type command and press Enter) ", iteration);
        fflush(stdout);

        SLEEP_MS(500);

#ifdef _WIN32
        if (_kbhit()) {
            fgets(input, sizeof(input), stdin);
#else
        fd_set fds;
        struct timeval tv;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        if (select(1, &fds, NULL, NULL, &tv) > 0) {
            fgets(input, sizeof(input), stdin);
#endif
            char cmd = input[0];

            switch (cmd) {
            case 'q':
            case 'Q':
                should_stop = 1;
                printf("\nShutting down...\n");
                break;

            case 'r':
            case 'R':
                printf("\nManually reloading config...\n");
                ret = idcu_config_reload();
                if (ret == IDCU_ERR_SUCCESS) {
                    printf("Config reloaded successfully!\n");
                    print_current_config();
                } else {
                    printf("Failed to reload config: %d\n", ret);
                }
                break;

            case 's':
            case 'S':
                printf("\nSaving config to file...\n");
                ret = idcu_config_save(config_file);
                if (ret == IDCU_ERR_SUCCESS) {
                    printf("Config saved successfully!\n");
                } else {
                    printf("Failed to save config: %d\n", ret);
                }
                break;

            case 'p':
            case 'P':
                print_current_config();
                break;

            case '\n':
                break;

            default:
                printf("\nUnknown command: %c\n", cmd);
                printf("Available commands: r, s, p, q\n");
            }
        }
    }

    idcu_config_watch_stop();
    idcu_config_shutdown();

    printf("\nExample completed!\n");
    printf("Total config changes detected: %d\n", config_changed_count);

    return 0;
}
