#include <idcu/storage/storage.h>
#include <stdio.h>

int main(void) {
    printf("=== Storage Transaction Example ===\n");
    
    idcu_storage_init();
    
    idcu_StorageDatabase* db;
    idcu_storage_open(":memory:", &db);
    
    printf("Creating table...\n");
    idcu_storage_execute(db, "CREATE TABLE accounts (id INT, balance INT)");
    
    printf("\nBeginning transaction...\n");
    int ret = idcu_storage_begin_transaction(db);
    if (ret != 0) {
        printf("Failed to begin transaction: %d\n", ret);
    } else {
        printf("Transaction started\n");
    }
    
    printf("\nInserting initial data...\n");
    idcu_storage_execute_format(db, "INSERT INTO accounts VALUES (%d, %d)", 1, 1000);
    idcu_storage_execute_format(db, "INSERT INTO accounts VALUES (%d, %d)", 2, 2000);
    
    printf("\nCommitting transaction...\n");
    ret = idcu_storage_commit_transaction(db);
    if (ret != 0) {
        printf("Failed to commit transaction: %d\n", ret);
    } else {
        printf("Transaction committed\n");
    }
    
    printf("\nBeginning another transaction...\n");
    idcu_storage_begin_transaction(db);
    
    printf("\nUpdating data...\n");
    idcu_storage_execute_format(db, "UPDATE accounts SET balance = %d WHERE id = %d", 1500, 1);
    
    printf("\nRolling back transaction...\n");
    ret = idcu_storage_rollback_transaction(db);
    if (ret != 0) {
        printf("Failed to rollback transaction: %d\n", ret);
    } else {
        printf("Transaction rolled back\n");
    }
    
    idcu_storage_close(db);
    idcu_storage_cleanup();
    
    printf("\n=== Example Complete ===\n");
    return 0;
}
