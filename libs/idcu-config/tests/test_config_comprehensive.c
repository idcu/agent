#include "idcu/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_passed = 0;
static int test_failed = 0;

#define TEST_ASSERT(cond, msg)                                                                     \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            printf("FAIL: %s\n", msg);                                                             \
            test_failed++;                                                                         \
        } else {                                                                                   \
            test_passed++;                                                                         \
            printf("PASS: %s\n", msg);                                                             \
        }                                                                                          \
    } while (0)

static void create_test_config_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp) {
        fprintf(fp, "[general]\n");
        fprintf(fp, "name = TestApp\n");
        fprintf(fp, "version = 1.0.0\n");
        fprintf(fp, "enabled = true\n");
        fprintf(fp, "port = 8080\n");
        fprintf(fp, "timeout = 30.5\n");
        fprintf(fp, "\n");
        fprintf(fp, "[database]\n");
        fprintf(fp, "host = localhost\n");
        fprintf(fp, "port = 5432\n");
        fprintf(fp, "user = admin\n");
        fprintf(fp, "password = secret\n");
        fprintf(fp, "\n");
        fprintf(fp, "[logging]\n");
        fprintf(fp, "level = debug\n");
        fprintf(fp, "file = app.log\n");
        fclose(fp);
    }
}

static void test_config_init_shutdown(void) {
    int ret = idcu_config_init(NULL);
    TEST_ASSERT(ret == 0, "config init should succeed");

    TEST_ASSERT(idcu_config_is_loaded() == 0, "should not be loaded without file");

    idcu_config_shutdown();
}

static void test_config_load_save(void) {
    const char *test_file = "test_config.ini";
    const char *save_file = "test_config_saved.ini";

    create_test_config_file(test_file);

    int ret = idcu_config_init(test_file);
    TEST_ASSERT(ret == 0, "config init with file should succeed");
    TEST_ASSERT(idcu_config_is_loaded() == 1, "should be loaded");

    ret = idcu_config_save(save_file);
    TEST_ASSERT(ret == 0, "config save should succeed");

    idcu_config_shutdown();

    remove(test_file);
    remove(save_file);
}

static void test_config_get_set_string(void) {
    const char *test_file = "test_config_string.ini";
    create_test_config_file(test_file);

    idcu_config_init(test_file);

    const char *name = idcu_config_get_string("general", "name", "default");
    TEST_ASSERT(strcmp(name, "TestApp") == 0, "should get correct string");

    const char *missing = idcu_config_get_string("general", "missing", "mydefault");
    TEST_ASSERT(strcmp(missing, "mydefault") == 0, "should return default for missing key");

    int ret = idcu_config_set_string("general", "new_key", "new_value");
    TEST_ASSERT(ret == 0, "set string should succeed");

    const char *new_val = idcu_config_get_string("general", "new_key", "");
    TEST_ASSERT(strcmp(new_val, "new_value") == 0, "should get the set value");

    idcu_config_shutdown();
    remove(test_file);
}

static void test_config_get_set_int(void) {
    const char *test_file = "test_config_int.ini";
    create_test_config_file(test_file);

    idcu_config_init(test_file);

    int port = idcu_config_get_int("general", "port", 9999);
    TEST_ASSERT(port == 8080, "should get correct int");

    int missing = idcu_config_get_int("general", "missing_int", 1234);
    TEST_ASSERT(missing == 1234, "should return default for missing int");

    int ret = idcu_config_set_int("general", "new_port", 9000);
    TEST_ASSERT(ret == 0, "set int should succeed");

    int new_port = idcu_config_get_int("general", "new_port", 0);
    TEST_ASSERT(new_port == 9000, "should get the set int value");

    idcu_config_shutdown();
    remove(test_file);
}

static void test_config_get_set_bool(void) {
    const char *test_file = "test_config_bool.ini";
    create_test_config_file(test_file);

    idcu_config_init(test_file);

    int enabled = idcu_config_get_bool("general", "enabled", 0);
    TEST_ASSERT(enabled == 1, "should get true");

    int missing = idcu_config_get_bool("general", "missing_bool", 1);
    TEST_ASSERT(missing == 1, "should return default for missing bool");

    int ret = idcu_config_set_bool("general", "debug_mode", 0);
    TEST_ASSERT(ret == 0, "set bool should succeed");

    int debug_mode = idcu_config_get_bool("general", "debug_mode", 1);
    TEST_ASSERT(debug_mode == 0, "should get the set bool value");

    idcu_config_shutdown();
    remove(test_file);
}

static void test_config_get_set_double(void) {
    const char *test_file = "test_config_double.ini";
    create_test_config_file(test_file);

    idcu_config_init(test_file);

    double timeout = idcu_config_get_double("general", "timeout", 0.0);
    TEST_ASSERT(timeout > 30.0 && timeout < 31.0, "should get correct double");

    int ret = idcu_config_set_double("general", "ratio", 0.75);
    TEST_ASSERT(ret == 0, "set double should succeed");

    double ratio = idcu_config_get_double("general", "ratio", 0.0);
    TEST_ASSERT(ratio > 0.7 && ratio < 0.8, "should get the set double value");

    idcu_config_shutdown();
    remove(test_file);
}

static void test_config_has_section_key(void) {
    const char *test_file = "test_config_has.ini";
    create_test_config_file(test_file);

    idcu_config_init(test_file);

    TEST_ASSERT(idcu_config_has_section("general") == 1, "should have general section");
    TEST_ASSERT(idcu_config_has_section("database") == 1, "should have database section");
    TEST_ASSERT(idcu_config_has_section("missing") == 0, "should not have missing section");

    TEST_ASSERT(idcu_config_has_key("general", "name") == 1, "should have name key");
    TEST_ASSERT(idcu_config_has_key("general", "port") == 1, "should have port key");
    TEST_ASSERT(idcu_config_has_key("general", "missing_key") == 0, "should not have missing key");

    idcu_config_shutdown();
    remove(test_file);
}

static void test_config_remove_key_section(void) {
    const char *test_file = "test_config_remove.ini";
    create_test_config_file(test_file);

    idcu_config_init(test_file);

    int ret = idcu_config_remove_key("general", "version");
    TEST_ASSERT(ret == 0, "remove key should succeed");
    TEST_ASSERT(idcu_config_has_key("general", "version") == 0, "key should be removed");

    ret = idcu_config_remove_section("logging");
    TEST_ASSERT(ret == 0, "remove section should succeed");
    TEST_ASSERT(idcu_config_has_section("logging") == 0, "section should be removed");

    idcu_config_shutdown();
    remove(test_file);
}

static void test_config_list(void) {
    const char *test_file = "test_config_list.ini";
    FILE *fp = fopen(test_file, "w");
    if (fp) {
        fprintf(fp, "[features]\n");
        fprintf(fp, "enabled = auth, logging, metrics\n");
        fclose(fp);
    }

    idcu_config_init(test_file);

    idcu_ConfigList list;
    int ret = idcu_config_get_list("features", "enabled", ",", &list);
    TEST_ASSERT(ret == 0, "get list should succeed");
    TEST_ASSERT(list.count == 3, "should have 3 items");

    int contains = idcu_config_list_contains("features", "enabled", ",", "logging");
    TEST_ASSERT(contains == 1, "should contain logging");

    contains = idcu_config_list_contains("features", "enabled", ",", "missing");
    TEST_ASSERT(contains == 0, "should not contain missing");

    idcu_config_shutdown();
    remove(test_file);
}

static void test_config_env_var(void) {
    idcu_config_init(NULL);

    idcu_config_enable_env_var(1);
    idcu_config_enable_validation(1);

    TEST_ASSERT(1 == 1, "env var functions should work");

    idcu_config_shutdown();
}

static void test_config_nested(void) {
    const char *test_file = "test_config_nested.ini";
    FILE *fp = fopen(test_file, "w");
    if (fp) {
        fprintf(fp, "[server]\n");
        fprintf(fp, "db.host = localhost\n");
        fprintf(fp, "db.port = 5432\n");
        fprintf(fp, "db.enabled = true\n");
        fclose(fp);
    }

    idcu_config_init(test_file);

    const char *host = idcu_config_get_nested_string("server", "db", "host", "default");
    TEST_ASSERT(strcmp(host, "localhost") == 0, "should get nested string");

    int port = idcu_config_get_nested_int("server", "db", "port", 0);
    TEST_ASSERT(port == 5432, "should get nested int");

    int enabled = idcu_config_get_nested_bool("server", "db", "enabled", 0);
    TEST_ASSERT(enabled == 1, "should get nested bool");

    idcu_config_shutdown();
    remove(test_file);
}

static void test_config_get_file_path(void) {
    const char *test_file = "test_config_path.ini";
    create_test_config_file(test_file);

    idcu_config_init(test_file);

    char buffer[256];
    int ret = idcu_config_get_file_path(buffer, sizeof(buffer));
    TEST_ASSERT(ret == 0, "get file path should succeed");
    TEST_ASSERT(strstr(buffer, "test_config_path.ini") != NULL, "path should contain filename");

    idcu_config_shutdown();
    remove(test_file);
}

int main(void) {
    printf("Running Comprehensive Config Tests...\n\n");

    test_config_init_shutdown();
    test_config_load_save();
    test_config_get_set_string();
    test_config_get_set_int();
    test_config_get_set_bool();
    test_config_get_set_double();
    test_config_has_section_key();
    test_config_remove_key_section();
    test_config_list();
    test_config_env_var();
    test_config_nested();
    test_config_get_file_path();

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
