#include "idcu/common/linked_list.h"
#include <stdlib.h>
#include <string.h>

int idcu_linked_list_init(idcu_LinkedList* list, size_t element_size)
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

int idcu_linked_list_init_with_dtor(idcu_LinkedList* list, size_t element_size,
                                       void (*element_dtor)(void*))
{
    int ret = idcu_linked_list_init(list, element_size);
    if (ret == IDCU_ERR_OK) {
        list->element_dtor = element_dtor;
    }
    return ret;
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

void idcu_linked_list_destroy(idcu_LinkedList* list)
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

int idcu_linked_list_push_front(idcu_LinkedList* list, const void* element)
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

int idcu_linked_list_push_back(idcu_LinkedList* list, const void* element)
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

int idcu_linked_list_pop_front(idcu_LinkedList* list, void* out_element)
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

int idcu_linked_list_pop_back(idcu_LinkedList* list, void* out_element)
{
    if (!list || !list->tail) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_LinkedListNode* node = list->tail;

    if (out_element) {
        memcpy(out_element, node->data, list->element_size);
    }

    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = node->prev;
        list->tail->next = NULL;
    }

    idcu_linked_list_destroy_node(list, node);
    list->size--;

    return IDCU_ERR_OK;
}

void* idcu_linked_list_front(const idcu_LinkedList* list)
{
    if (!list || !list->head) {
        return NULL;
    }
    return list->head->data;
}

void* idcu_linked_list_back(const idcu_LinkedList* list)
{
    if (!list || !list->tail) {
        return NULL;
    }
    return list->tail->data;
}

size_t idcu_linked_list_size(const idcu_LinkedList* list) { return list ? list->size : 0; }
bool idcu_linked_list_empty(const idcu_LinkedList* list) { return list ? (list->size == 0) : true; }

void idcu_linked_list_clear(idcu_LinkedList* list)
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

void idcu_linked_list_iterator_init(idcu_LinkedListIterator* iter, idcu_LinkedList* list)
{
    if (!iter || !list) {
        return;
    }
    iter->list = list;
    iter->current = list->head;
}

bool idcu_linked_list_iterator_next(idcu_LinkedListIterator* iter, void** out_value)
{
    if (!iter || !iter->list || !iter->current) {
        return false;
    }

    if (out_value) {
        *out_value = iter->current->data;
    }

    iter->current = iter->current->next;
    return true;
}
