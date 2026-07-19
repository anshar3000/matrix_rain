#include "list.h"
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

static linked_list_node *node_alloc(linked_list_t l, void *data) {
    linked_list_node *n =
        allocator_alloc(l->allocator, sizeof(linked_list_node),
                        _Alignof(linked_list_node));
    if (n == NULL)
        return NULL;
    n->data = data;
    n->next = NULL;
    n->prev = NULL;
    return n;
}

static void node_free(linked_list_t l, linked_list_node *n) {
    allocator_free(l->allocator, n, sizeof(linked_list_node),
                   _Alignof(linked_list_node));
}

/* Unlink and free a node; does NOT update l->size. */
static void unlink_node(linked_list_t l, linked_list_node *n) {
    if (n->prev != NULL)
        n->prev->next = n->next;
    else
        l->head = n->next;

    if (n->next != NULL)
        n->next->prev = n->prev;
    else
        l->tail = n->prev;

    node_free(l, n);
}

/* -------------------------------------------------------------------------
 * Init / free
 * ---------------------------------------------------------------------- */

linked_list_t linked_list_init(allocator_t allocator) {
    linked_list_t l = malloc(sizeof(linked_list));
    if (l == NULL)
        return NULL;

    l->head      = NULL;
    l->tail      = NULL;
    l->size      = 0;
    l->allocator = allocator;
    return l;
}

void linked_list_free(linked_list_t l) {
    if (l == NULL)
        return;

    linked_list_node *cur = l->head;
    while (cur != NULL) {
        linked_list_node *next = cur->next;
        node_free(l, cur);
        cur = next;
    }
    free(l);
}

/* -------------------------------------------------------------------------
 * Push / pop
 * ---------------------------------------------------------------------- */

ll_result_t linked_list_push_back(linked_list_t l, void *data) {
    if (l == NULL)
        return ll_result_err(LL_ERR_NULL_LIST);

    linked_list_node *n = node_alloc(l, data);
    if (n == NULL)
        return ll_result_err(LL_ERR_ALLOC);

    n->prev = l->tail;
    if (l->tail != NULL)
        l->tail->next = n;
    else
        l->head = n;
    l->tail = n;
    l->size++;
    return ll_result_ok();
}

ll_result_t linked_list_push_front(linked_list_t l, void *data) {
    if (l == NULL)
        return ll_result_err(LL_ERR_NULL_LIST);

    linked_list_node *n = node_alloc(l, data);
    if (n == NULL)
        return ll_result_err(LL_ERR_ALLOC);

    n->next = l->head;
    if (l->head != NULL)
        l->head->prev = n;
    else
        l->tail = n;
    l->head = n;
    l->size++;
    return ll_result_ok();
}

ll_maybe_t linked_list_pop_back(linked_list_t l) {
    if (l == NULL)
        return ll_maybe_err(LL_ERR_NULL_LIST);
    if (l->tail == NULL)
        return ll_maybe_err(LL_ERR_OUT_OF_BOUNDS);

    linked_list_node *n    = l->tail;
    void             *data = n->data;

    l->tail = n->prev;
    if (l->tail != NULL)
        l->tail->next = NULL;
    else
        l->head = NULL;

    node_free(l, n);
    l->size--;
    return ll_maybe_ok(data);
}

ll_maybe_t linked_list_pop_front(linked_list_t l) {
    if (l == NULL)
        return ll_maybe_err(LL_ERR_NULL_LIST);
    if (l->head == NULL)
        return ll_maybe_err(LL_ERR_OUT_OF_BOUNDS);

    linked_list_node *n    = l->head;
    void             *data = n->data;

    l->head = n->next;
    if (l->head != NULL)
        l->head->prev = NULL;
    else
        l->tail = NULL;

    node_free(l, n);
    l->size--;
    return ll_maybe_ok(data);
}

/* -------------------------------------------------------------------------
 * Queries
 * ---------------------------------------------------------------------- */

size_t linked_list_size(linked_list_t l) {
    return (l == NULL) ? 0 : l->size;
}

bool linked_list_is_empty(linked_list_t l) {
    return (l == NULL) || (l->size == 0);
}

/* -------------------------------------------------------------------------
 * Clear
 * ---------------------------------------------------------------------- */

void linked_list_clear(linked_list_t l) {
    if (l == NULL)
        return;

    linked_list_node *cur = l->head;
    while (cur != NULL) {
        linked_list_node *next = cur->next;
        node_free(l, cur);
        cur = next;
    }
    l->head = NULL;
    l->tail = NULL;
    l->size = 0;
}

/* -------------------------------------------------------------------------
 * Higher-order traversal
 * ---------------------------------------------------------------------- */

ll_result_t linked_list_for_each(linked_list_t l,
                                  void (*func)(void *, void *),
                                  void *context) {
    if (l == NULL)
        return ll_result_err(LL_ERR_NULL_LIST);
    if (func == NULL)
        return ll_result_err(LL_ERR_NULL_FUNC);

    for (linked_list_node *cur = l->head; cur != NULL; cur = cur->next)
        func(context, cur->data);

    return ll_result_ok();
}

ll_result_t linked_list_map(linked_list_t l,
                             void *(*func)(void *, void *),
                             void *context) {
    if (l == NULL)
        return ll_result_err(LL_ERR_NULL_LIST);
    if (func == NULL)
        return ll_result_err(LL_ERR_NULL_FUNC);

    for (linked_list_node *cur = l->head; cur != NULL; cur = cur->next)
        cur->data = func(context, cur->data);

    return ll_result_ok();
}

/* -------------------------------------------------------------------------
 * Index-based access
 * ---------------------------------------------------------------------- */

ll_maybe_t linked_list_get_at(linked_list_t l, size_t index) {
    if (l == NULL)
        return ll_maybe_err(LL_ERR_NULL_LIST);
    if (index >= l->size)
        return ll_maybe_err(LL_ERR_OUT_OF_BOUNDS);

    linked_list_node *cur = l->head;
    for (size_t i = 0; i < index; i++)
        cur = cur->next;

    return ll_maybe_ok(cur->data);
}

ll_result_t linked_list_remove_at(linked_list_t l, size_t index) {
    if (l == NULL)
        return ll_result_err(LL_ERR_NULL_LIST);
    if (index >= l->size)
        return ll_result_err(LL_ERR_OUT_OF_BOUNDS);

    linked_list_node *cur = l->head;
    for (size_t i = 0; i < index; i++)
        cur = cur->next;

    unlink_node(l, cur);
    l->size--;
    return ll_result_ok();
}

ll_result_t linked_list_insert_at(linked_list_t l, size_t index, void *data) {
    if (l == NULL)
        return ll_result_err(LL_ERR_NULL_LIST);
    if (index > l->size)
        return ll_result_err(LL_ERR_OUT_OF_BOUNDS);

    if (index == 0)
        return linked_list_push_front(l, data);
    if (index == l->size)
        return linked_list_push_back(l, data);

    linked_list_node *n = node_alloc(l, data);
    if (n == NULL)
        return ll_result_err(LL_ERR_ALLOC);

    linked_list_node *cur = l->head;
    for (size_t i = 0; i < index; i++)
        cur = cur->next;

    n->next = cur;
    n->prev = cur->prev;
    if (cur->prev != NULL)
        cur->prev->next = n;
    cur->prev = n;
    l->size++;
    return ll_result_ok();
}

/* -------------------------------------------------------------------------
 * Structural operations
 * ---------------------------------------------------------------------- */

ll_result_t linked_list_reverse(linked_list_t l) {
    if (l == NULL)
        return ll_result_err(LL_ERR_NULL_LIST);
    if (l->size <= 1)
        return ll_result_ok();

    linked_list_node *cur  = l->head;
    linked_list_node *temp = NULL;

    while (cur != NULL) {
        temp      = cur->prev;
        cur->prev = cur->next;
        cur->next = temp;
        cur       = cur->prev; /* advance via the old next */
    }

    temp   = l->head;
    l->head = l->tail;
    l->tail = temp;
    return ll_result_ok();
}

linked_list_t linked_list_copy(linked_list_t l) {
    if (l == NULL)
        return NULL;

    linked_list_t copy = linked_list_init(l->allocator);
    if (copy == NULL)
        return NULL;

    for (linked_list_node *cur = l->head; cur != NULL; cur = cur->next) {
        ll_result_t r = linked_list_push_back(copy, cur->data);
        if (r.err != LL_OK) {
            linked_list_free(copy);
            return NULL;
        }
    }
    return copy;
}

linked_list_t linked_list_filter(linked_list_t l,
                                  bool (*predicate)(void *, void *),
                                  void *context) {
    if (l == NULL || predicate == NULL)
        return NULL;

    linked_list_t result = linked_list_init(l->allocator);
    if (result == NULL)
        return NULL;

    for (linked_list_node *cur = l->head; cur != NULL; cur = cur->next) {
        if (predicate(context, cur->data)) {
            ll_result_t r = linked_list_push_back(result, cur->data);
            if (r.err != LL_OK) {
                linked_list_free(result);
                return NULL;
            }
        }
    }
    return result;
}

ll_result_t linked_list_remove_if(linked_list_t l,
                                   bool (*predicate)(void *, void *),
                                   void *context) {
    if (l == NULL)
        return ll_result_err(LL_ERR_NULL_LIST);
    if (predicate == NULL)
        return ll_result_err(LL_ERR_NULL_FUNC);

    linked_list_node *cur = l->head;
    while (cur != NULL) {
        linked_list_node *next = cur->next;
        if (predicate(context, cur->data)) {
            unlink_node(l, cur); /* handles head/tail fixup */
            l->size--;
        }
        cur = next;
    }
    return ll_result_ok();
}
