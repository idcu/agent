#include "idcu/common/linked_list.h"
#include <stdlib.h>
#include <string.h>

static idcu_ListNode *idcu_list_node_create(const void *element, size_t element_size) {
    idcu_ListNode *node = (idcu_ListNode *)malloc(sizeof(idcu_ListNode));
    if (!node) {
        return NULL;
    }

    node->data = malloc(element_size);
    if (!node->data) {
        free(node);
        return NULL;
    }

    memcpy(node->data, element, element_size);
    node->prev = NULL;
    node->next = NULL;
    return node;
}

static void idcu_list_node_destroy(idcu_ListNode *node, void (*element_dtor)(void *)) {
    if (!node)
        return;
    if (element_dtor) {
        element_dtor(node->data);
    }
    free(node->data);
    free(node);
}

int idcu_linked_list_init(idcu_LinkedList *list, size_t element_size) {
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

int idcu_linked_list_init_with_dtor(idcu_LinkedList *list, size_t element_size,
                                    void (*element_dtor)(void *)) {
    int ret = idcu_linked_list_init(list, element_size);
    if (ret == IDCU_ERR_OK) {
        list->element_dtor = element_dtor;
    }
    return ret;
}

void idcu_linked_list_destroy(idcu_LinkedList *list) {
    if (!list)
        return;
    idcu_linked_list_clear(list);
}

int idcu_linked_list_push_front(idcu_LinkedList *list, const void *element) {
    if (!list || !element) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ListNode *new_node = idcu_list_node_create(element, list->element_size);
    if (!new_node) {
        return IDCU_ERR_NO_MEMORY;
    }

    if (list->size == 0) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
    }

    list->size++;
    return IDCU_ERR_OK;
}

int idcu_linked_list_push_back(idcu_LinkedList *list, const void *element) {
    if (!list || !element) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ListNode *new_node = idcu_list_node_create(element, list->element_size);
    if (!new_node) {
        return IDCU_ERR_NO_MEMORY;
    }

    if (list->size == 0) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->tail = new_node;
    }

    list->size++;
    return IDCU_ERR_OK;
}

int idcu_linked_list_pop_front(idcu_LinkedList *list, void *out_element) {
    if (!list || list->size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ListNode *node = list->head;
    if (out_element) {
        memcpy(out_element, node->data, list->element_size);
    }

    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = node->next;
        list->head->prev = NULL;
    }

    idcu_list_node_destroy(node, list->element_dtor);
    list->size--;
    return IDCU_ERR_OK;
}

int idcu_linked_list_pop_back(idcu_LinkedList *list, void *out_element) {
    if (!list || list->size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ListNode *node = list->tail;
    if (out_element) {
        memcpy(out_element, node->data, list->element_size);
    }

    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = node->prev;
        list->tail->next = NULL;
    }

    idcu_list_node_destroy(node, list->element_dtor);
    list->size--;
    return IDCU_ERR_OK;
}

static idcu_ListNode *idcu_list_node_at(const idcu_LinkedList *list, size_t index) {
    if (!list || index >= list->size) {
        return NULL;
    }

    idcu_ListNode *node;
    if (index < list->size / 2) {
        node = list->head;
        for (size_t i = 0; i < index; i++) {
            node = node->next;
        }
    } else {
        node = list->tail;
        for (size_t i = list->size - 1; i > index; i--) {
            node = node->prev;
        }
    }
    return node;
}

int idcu_linked_list_insert_before(idcu_LinkedList *list, size_t index, const void *element) {
    if (!list || !element || index > list->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (index == 0) {
        return idcu_linked_list_push_front(list, element);
    }

    if (index == list->size) {
        return idcu_linked_list_push_back(list, element);
    }

    idcu_ListNode *next_node = idcu_list_node_at(list, index);
    if (!next_node) {
        return IDCU_ERR_GENERAL;
    }

    idcu_ListNode *new_node = idcu_list_node_create(element, list->element_size);
    if (!new_node) {
        return IDCU_ERR_NO_MEMORY;
    }

    new_node->prev = next_node->prev;
    new_node->next = next_node;
    next_node->prev->next = new_node;
    next_node->prev = new_node;
    list->size++;
    return IDCU_ERR_OK;
}

int idcu_linked_list_insert_after(idcu_LinkedList *list, size_t index, const void *element) {
    if (!list || !element || index >= list->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (index == list->size - 1) {
        return idcu_linked_list_push_back(list, element);
    }

    idcu_ListNode *prev_node = idcu_list_node_at(list, index);
    if (!prev_node) {
        return IDCU_ERR_GENERAL;
    }

    idcu_ListNode *new_node = idcu_list_node_create(element, list->element_size);
    if (!new_node) {
        return IDCU_ERR_NO_MEMORY;
    }

    new_node->prev = prev_node;
    new_node->next = prev_node->next;
    prev_node->next->prev = new_node;
    prev_node->next = new_node;
    list->size++;
    return IDCU_ERR_OK;
}

int idcu_linked_list_remove(idcu_LinkedList *list, size_t index) {
    if (!list || index >= list->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (index == 0) {
        return idcu_linked_list_pop_front(list, NULL);
    }

    if (index == list->size - 1) {
        return idcu_linked_list_pop_back(list, NULL);
    }

    idcu_ListNode *node = idcu_list_node_at(list, index);
    if (!node) {
        return IDCU_ERR_GENERAL;
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;
    idcu_list_node_destroy(node, list->element_dtor);
    list->size--;
    return IDCU_ERR_OK;
}

void *idcu_linked_list_get(const idcu_LinkedList *list, size_t index) {
    idcu_ListNode *node = idcu_list_node_at(list, index);
    return node ? node->data : NULL;
}

void *idcu_linked_list_front(const idcu_LinkedList *list) {
    if (!list || list->size == 0) {
        return NULL;
    }
    return list->head->data;
}

void *idcu_linked_list_back(const idcu_LinkedList *list) {
    if (!list || list->size == 0) {
        return NULL;
    }
    return list->tail->data;
}

size_t idcu_linked_list_size(const idcu_LinkedList *list) { return list ? list->size : 0; }

bool idcu_linked_list_empty(const idcu_LinkedList *list) { return list ? (list->size == 0) : true; }

void idcu_linked_list_clear(idcu_LinkedList *list) {
    if (!list)
        return;

    idcu_ListNode *node = list->head;
    while (node) {
        idcu_ListNode *next = node->next;
        idcu_list_node_destroy(node, list->element_dtor);
        node = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void idcu_linked_list_iter_init(idcu_LinkedListIterator *iter, idcu_LinkedList *list) {
    if (!iter || !list)
        return;
    iter->list = list;
    iter->current = NULL;
    iter->direction = 1;
}

void idcu_linked_list_iter_init_reverse(idcu_LinkedListIterator *iter, idcu_LinkedList *list) {
    if (!iter || !list)
        return;
    iter->list = list;
    iter->current = NULL;
    iter->direction = -1;
}

bool idcu_linked_list_iter_next(idcu_LinkedListIterator *iter, void **out_element) {
    if (!iter || !iter->list)
        return false;

    if (!iter->current) {
        if (iter->direction == 1) {
            iter->current = iter->list->head;
        } else {
            iter->current = iter->list->tail;
        }
    } else {
        if (iter->direction == 1) {
            iter->current = iter->current->next;
        } else {
            iter->current = iter->current->prev;
        }
    }

    if (!iter->current) {
        return false;
    }

    if (out_element) {
        *out_element = iter->current->data;
    }
    return true;
}

void idcu_linked_list_iter_destroy(idcu_LinkedListIterator *iter) {
    if (!iter)
        return;
    iter->list = NULL;
    iter->current = NULL;
    iter->direction = 0;
}
