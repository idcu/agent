#include <idcu/storage/storage.h>
#include <stdio.h>

int main(void) {
    printf("=== Basic Storage Example ===\n");

    idcu_storage_init();

    idcu_StorageDatabase *db;
    int ret = idcu_storage_open(":memory:", &db);
    if (ret != 0) {
        printf("Failed to open database: %d\n", ret);
        idcu_storage_cleanup();
        return 1;
    }
    printf("Database opened successfully\n");

    printf("\nCreating table...\n");
    ret = idcu_storage_execute(db, "CREATE TABLE users (id INT PRIMARY KEY, name TEXT, age INT)");
    if (ret != 0) {
        printf("Failed to create table: %d\n", ret);
    } else {
        printf("Table created successfully\n");
    }

    printf("\nInserting data...\n");
    ret =
        idcu_storage_execute_format(db, "INSERT INTO users VALUES (%d, '%s', %d)", 1, "Alice", 30);
    if (ret != 0) {
        printf("Failed to insert data: %d\n", ret);
    } else {
        printf("Data inserted successfully\n");
    }

    printf("\nClosing database...\n");
    ret = idcu_storage_close(db);
    if (ret != 0) {
        printf("Failed to close database: %d\n", ret);
    } else {
        printf("Database closed successfully\n");
    }

    idcu_storage_cleanup();

    printf("\n=== Example Complete ===\n");
    return 0;
}
