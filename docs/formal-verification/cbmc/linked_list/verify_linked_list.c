/**
 * @file verify_linked_list.c
 * @brief CBMC 形式化验证 - 双向链表
 *
 * 验证属性:
 * 1. 链表不变式: head->prev == NULL, tail->next == NULL
 * 2. 节点一致性: 对于每个节点 n, n->next->prev == n
 * 3. 大小一致性: list->size == 实际节点数
 * 4. 无循环引用: 从 head 遍历不会回到已访问的节点
 * 5. 内存安全: 所有指针操作都是安全的
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define IDCU_ERR_OK 0
#define IDCU_ERR_INVALID_PARAM 1
#define IDCU_ERR_NO_MEMORY 2

typedef struct idcu_LinkedListNode
{
    struct idcu_LinkedListNode* prev;
    struct idcu_LinkedListNode* next;
    void* data;
} idcu_LinkedListNode;

typedef struct
{
    idcu_LinkedListNode* head;
    idcu_LinkedListNode* tail;
    size_t size;
    size_t element_size;
    void (*element_dtor)(void*);
} idcu_LinkedList;

static int idcu_linked_list_init(idcu_LinkedList* list, size_t element_size)
{
    if (!list || element_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->element_size = element_size;
    list->element_dtor = NULL;

    return IDCU_ERR_OK;
}

static void idcu_linked_list_destroy_node(idcu_LinkedList* list, idcu_LinkedListNode* node)
{
    if (!node)
        return;
    if (list->element_dtor) {
        list->element_dtor(node->data);
    }
    free(node->data);
    free(node);
}

static void idcu_linked_list_destroy(idcu_LinkedList* list)
{
    if (!list)
        return;

    idcu_LinkedListNode* node = list->head;
    while (node) {
        idcu_LinkedListNode* next = node->next;
        idcu_linked_list_destroy_node(list, node);
        node = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

static idcu_LinkedListNode* idcu_linked_list_create_node(idcu_LinkedList* list, const void* element)
{
    idcu_LinkedListNode* node = (idcu_LinkedListNode*)malloc(sizeof(idcu_LinkedListNode));
    if (!node) {
        return NULL;
    }

    node->data = malloc(list->element_size);
    if (!node->data) {
        free(node);
        return NULL;
    }

    memcpy(node->data, element, list->element_size);
    node->prev = NULL;
    node->next = NULL;

    return node;
}

static int idcu_linked_list_push_front(idcu_LinkedList* list, const void* element)
{
    if (!list || !element) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_LinkedListNode* node = idcu_linked_list_create_node(list, element);
    if (!node) {
        return IDCU_ERR_NO_MEMORY;
    }

    if (list->head) {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    } else {
        list->head = node;
        list->tail = node;
    }

    list->size++;
    return IDCU_ERR_OK;
}

static int idcu_linked_list_push_back(idcu_LinkedList* list, const void* element)
{
    if (!list || !element) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_LinkedListNode* node = idcu_linked_list_create_node(list, element);
    if (!node) {
        return IDCU_ERR_NO_MEMORY;
    }

    if (list->tail) {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    } else {
        list->head = node;
        list->tail = node;
    }

    list->size++;
    return IDCU_ERR_OK;
}

static int idcu_linked_list_pop_front(idcu_LinkedList* list, void* out_element)
{
    if (!list || !list->head) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_LinkedListNode* node = list->head;

    if (out_element) {
        memcpy(out_element, node->data, list->element_size);
    }

    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = node->next;
        list->head->prev = NULL;
    }

    idcu_linked_list_destroy_node(list, node);
    list->size--;

    return IDCU_ERR_OK;
}

static size_t idcu_linked_list_size(const idcu_LinkedList* list) { 
    return list ? list->size : 0; 
}

static bool idcu_linked_list_empty(const idcu_LinkedList* list) { 
    return list ? (list->size == 0) : true; 
}

bool verify_linked_list_invariant(const idcu_LinkedList* list)
{
    if (!list) return true;

    if (list->size == 0) {
        return (list->head == NULL && list->tail == NULL);
    }

    if (list->head == NULL || list->tail == NULL) {
        return false;
    }

    if (list->head->prev != NULL) {
        return false;
    }

    if (list->tail->next != NULL) {
        return false;
    }

    size_t count = 0;
    idcu_LinkedListNode* current = list->head;
    idcu_LinkedListNode* prev = NULL;

    while (current != NULL) {
        count++;

        if (current->prev != prev) {
            return false;
        }

        prev = current;
        current = current->next;
    }

    if (prev != list->tail) {
        return false;
    }

    if (count != list->size) {
        return false;
    }

    return true;
}

void test_linked_list_init()
{
    idcu_LinkedList list;
    int ret = idcu_linked_list_init(&list, sizeof(int));
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "初始化应该成功");
    __CPROVER_assert(list.head == NULL, "head 应该为 NULL");
    __CPROVER_assert(list.tail == NULL, "tail 应该为 NULL");
    __CPROVER_assert(list.size == 0, "size 应该为 0");
    __CPROVER_assert(verify_linked_list_invariant(&list), "链表不变式应该成立");
    
    idcu_linked_list_destroy(&list);
}

void test_linked_list_push_front()
{
    idcu_LinkedList list;
    idcu_linked_list_init(&list, sizeof(int));
    
    int value = 42;
    int ret = idcu_linked_list_push_front(&list, &value);
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "push_front 应该成功");
    __CPROVER_assert(list.size == 1, "size 应该为 1");
    __CPROVER_assert(list.head != NULL, "head 不应该为 NULL");
    __CPROVER_assert(list.tail != NULL, "tail 不应该为 NULL");
    __CPROVER_assert(list.head == list.tail, "head 和 tail 应该相同");
    __CPROVER_assert(verify_linked_list_invariant(&list), "链表不变式应该成立");
    
    int value2 = 100;
    ret = idcu_linked_list_push_front(&list, &value2);
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "第二次 push_front 应该成功");
    __CPROVER_assert(list.size == 2, "size 应该为 2");
    __CPROVER_assert(list.head != list.tail, "head 和 tail 应该不同");
    __CPROVER_assert(verify_linked_list_invariant(&list), "链表不变式应该成立");
    
    idcu_linked_list_destroy(&list);
}

void test_linked_list_push_back()
{
    idcu_LinkedList list;
    idcu_linked_list_init(&list, sizeof(int));
    
    int value = 42;
    int ret = idcu_linked_list_push_back(&list, &value);
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "push_back 应该成功");
    __CPROVER_assert(list.size == 1, "size 应该为 1");
    __CPROVER_assert(verify_linked_list_invariant(&list), "链表不变式应该成立");
    
    idcu_linked_list_destroy(&list);
}

void test_linked_list_pop_front()
{
    idcu_LinkedList list;
    idcu_linked_list_init(&list, sizeof(int));
    
    int value = 42;
    idcu_linked_list_push_front(&list, &value);
    
    int out;
    int ret = idcu_linked_list_pop_front(&list, &out);
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "pop_front 应该成功");
    __CPROVER_assert(out == 42, "弹出的值应该正确");
    __CPROVER_assert(list.size == 0, "size 应该为 0");
    __CPROVER_assert(verify_linked_list_invariant(&list), "链表不变式应该成立");
    
    idcu_linked_list_destroy(&list);
}

void test_linked_list_multiple_operations()
{
    idcu_LinkedList list;
    idcu_linked_list_init(&list, sizeof(int));
    
    __CPROVER_assert(verify_linked_list_invariant(&list), "初始状态不变式成立");
    
    int v1 = 1, v2 = 2, v3 = 3;
    
    idcu_linked_list_push_back(&list, &v1);
    __CPROVER_assert(verify_linked_list_invariant(&list), "push_back 后不变式成立");
    
    idcu_linked_list_push_front(&list, &v2);
    __CPROVER_assert(verify_linked_list_invariant(&list), "push_front 后不变式成立");
    
    idcu_linked_list_push_back(&list, &v3);
    __CPROVER_assert(verify_linked_list_invariant(&list), "第二次 push_back 后不变式成立");
    
    __CPROVER_assert(list.size == 3, "size 应该为 3");
    
    int out;
    idcu_linked_list_pop_front(&list, &out);
    __CPROVER_assert(verify_linked_list_invariant(&list), "pop_front 后不变式成立");
    
    idcu_linked_list_destroy(&list);
}

int main()
{
    test_linked_list_init();
    test_linked_list_push_front();
    test_linked_list_push_back();
    test_linked_list_pop_front();
    test_linked_list_multiple_operations();
    
    return 0;
}
