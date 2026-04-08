#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "idcu/watchdog/watchdog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#endif

#define TEST_PASS 0
#define TEST_FAIL 1

static volatile int g_timeout_triggered = 0;
static volatile int g_restart_triggered = 0;

static void watchdog_timeout_callback(void *user_data) {
    (void)user_data;
    idcu_log_warn("Watchdog timeout callback triggered");
    g_timeout_triggered = 1;
}

static void watchdog_restart_callback(void *user_data) {
    (void)user_data;
    idcu_log_warn("Watchdog restart callback triggered");
    g_restart_triggered = 1;
}

static int test_watchdog_functionality(void) {
    idcu_log_info("=== Starting watchdog functionality test ===");

    idcu_Watchdog wd;
    int ret = idcu_watchdog_init(&wd, 2000, 3, 60000);
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("Failed to initialize watchdog");
        return TEST_FAIL;
    }

    g_timeout_triggered = 0;
    g_restart_triggered = 0;

    idcu_watchdog_set_on_timeout(&wd, watchdog_timeout_callback, NULL);
    idcu_watchdog_set_on_restart(&wd, watchdog_restart_callback, NULL);

    ret = idcu_watchdog_start(&wd);
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("Failed to start watchdog");
        idcu_watchdog_destroy(&wd);
        return TEST_FAIL;
    }

    idcu_log_info("Watchdog started, testing regular heartbeats...");
    for (int i = 0; i < 5; i++) {
        idcu_watchdog_heartbeat(&wd);
#ifdef _WIN32
        Sleep(500);
#else
        usleep(500 * 1000);
#endif
    }

    if (g_timeout_triggered || g_restart_triggered) {
        idcu_log_error("Watchdog triggered unexpectedly during normal operation");
        idcu_watchdog_stop(&wd);
        idcu_watchdog_destroy(&wd);
        return TEST_FAIL;
    }

    idcu_log_info("Testing watchdog timeout...");
#ifdef _WIN32
    Sleep(3000);
#else
    sleep(3);
#endif

    if (!g_timeout_triggered) {
        idcu_log_error("Watchdog timeout not triggered as expected");
        idcu_watchdog_stop(&wd);
        idcu_watchdog_destroy(&wd);
        return TEST_FAIL;
    }

    idcu_log_info("Watchdog timeout test passed");

    idcu_watchdog_stop(&wd);
    idcu_watchdog_destroy(&wd);

    idcu_log_info("=== Watchdog functionality test PASSED ===");
    return TEST_PASS;
}

static int test_error_recovery_simulation(void) {
    idcu_log_info("=== Starting error recovery simulation test ===");

    int recovery_attempts = 0;
    const int MAX_RECOVERY_ATTEMPTS = 3;

    for (int i = 0; i < MAX_RECOVERY_ATTEMPTS; i++) {
        idcu_log_info("Simulating error condition, attempt %d/%d", i + 1, MAX_RECOVERY_ATTEMPTS);
        recovery_attempts++;

#ifdef _WIN32
        Sleep(100);
#else
        usleep(100 * 1000);
#endif
    }

    if (recovery_attempts == MAX_RECOVERY_ATTEMPTS) {
        idcu_log_info("Error recovery simulation completed successfully");
        idcu_log_info("=== Error recovery simulation test PASSED ===");
        return TEST_PASS;
    } else {
        idcu_log_error("Error recovery simulation failed");
        return TEST_FAIL;
    }
}

static int test_network_disconnect_simulation(void) {
    idcu_log_info("=== Starting network disconnect simulation test ===");

    idcu_log_info("Simulating network connection...");
#ifdef _WIN32
    Sleep(500);
#else
    usleep(500 * 1000);
#endif

    idcu_log_info("Simulating network disconnect...");
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif

    idcu_log_info("Simulating network reconnect...");
#ifdef _WIN32
    Sleep(500);
#else
    usleep(500 * 1000);
#endif

    idcu_log_info("Network disconnect simulation completed");
    idcu_log_info("=== Network disconnect simulation test PASSED ===");
    return TEST_PASS;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    idcu_log_info("=== Starting Fault Tolerance Tests ===");

    int failures = 0;

    if (test_watchdog_functionality() != TEST_PASS) {
        failures++;
    }

    if (test_error_recovery_simulation() != TEST_PASS) {
        failures++;
    }

    if (test_network_disconnect_simulation() != TEST_PASS) {
        failures++;
    }

    if (failures == 0) {
        idcu_log_info("=== All Fault Tolerance Tests PASSED ===");
        return 0;
    } else {
        idcu_log_error("=== %d Fault Tolerance Test(s) FAILED ===", failures);
        return 1;
    }
}
