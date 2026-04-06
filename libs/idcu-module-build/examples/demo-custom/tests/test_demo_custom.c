#include "demo_custom/demo_custom.h"
#include <stdio.h>
#include <assert.h>

int main(void)
{
    printf("Running demo-custom tests...\n");
    
    int ret = custom_init();
    assert(ret == 0);
    printf("Test 1 passed: init\n");
    
    CustomData data;
    strncpy(data.name, "test", sizeof(data.name));
    data.value = 42;
    
    ret = custom_process(&data);
    assert(ret == 0);
    printf("Test 2 passed: process\n");
    assert(data.timestamp > 0);
    printf("Test 3 passed: timestamp set\n");
    
    ret = custom_cleanup();
    assert(ret == 0);
    printf("Test 4 passed: cleanup\n");
    
    printf("All tests passed!\n");
    return 0;
}
