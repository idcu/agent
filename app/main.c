#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"

static void print_version(void) {
    printf("IDCU Agent %s\n", idcu_agent_get_version());
}

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n");
    printf("\n");
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
            print_version();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Unknown option: %s\n", arg);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    printf("========================================\n");
    print_version();
    printf("========================================\n");
    printf("Hello, IDCU Agent!\n");
    printf("System is starting...\n");

    printf("System started successfully!\n");
    printf("Press Ctrl+C to exit...\n");

    while (1) {
    }

    return EXIT_SUCCESS;
}
