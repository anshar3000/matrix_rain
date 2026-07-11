#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

#include <allocator.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * Error / result types
 * ---------------------------------------------------------------------- */

typedef enum {
    LL_OK = 0,
    LL_ERR_NULL_LIST,       /* l == NULL                          */
    LL_ERR_NULL_FUNC,       /* callback / predicate == NULL       */
    LL_ERR_OUT_OF_BOUNDS,   /* index >= size (or > size for insert) */
    LL_ERR_ALLOC,           /* node allocation failed             */
} ll_err_t;

/* For functions that return void on success. */
typedef struct {
    ll_err_t err;
} ll_result_t;

/* For functions that return a data pointer on success. */
typedef struct {
    ll_err_t  err;
    void     *value; /* valid only when err == LL_OK */
} ll_maybe_t;

/* Convenience constructors */
static inline ll_result_t ll_result_ok(void)           { return (ll_result_t){ LL_OK }; }
static inline ll_result_t ll_result_err(ll_err_t e)    { return (ll_result_t){ e }; }
static inline ll_maybe_t  ll_maybe_ok(void *v)         { return (ll_maybe_t){ LL_OK, v }; }
static inline ll_maybe_t  ll_maybe_err(ll_err_t e)     { return (ll_maybe_t){ e, NULL }; }

/* -------------------------------------------------------------------------
 * Core types
 * ---------------------------------------------------------------------- */

typedef struct linked_list_node {
    void                    *data;
    struct linked_list_node *next;
    struct linked_list_node *prev;
} linked_list_node;

typedef struct linked_list {
    linked_list_node *head;
    linked_list_node *tail;
    size_t            size;
    allocator_t       allocator; /* used for node allocation */
} linked_list;

typedef linked_list *linked_list_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/* The linked_list struct itself is always malloc'd; only nodes use allocator. */
linked_list_t linked_list_init(allocator_t allocator);
void          linked_list_free(linked_list_t l);

ll_result_t linked_list_push_back (linked_list_t l, void *data);
ll_result_t linked_list_push_front(linked_list_t l, void *data);
ll_maybe_t  linked_list_pop_back  (linked_list_t l);
ll_maybe_t  linked_list_pop_front (linked_list_t l);

size_t linked_list_size    (linked_list_t l);
bool   linked_list_is_empty(linked_list_t l);
void   linked_list_clear   (linked_list_t l);

ll_result_t linked_list_for_each (linked_list_t l, void  (*func)(void *, void *), void *context);
ll_result_t linked_list_map      (linked_list_t l, void *(*func)(void *, void *), void *context);

ll_maybe_t  linked_list_get_at   (linked_list_t l, size_t index);
ll_result_t linked_list_remove_at(linked_list_t l, size_t index);
ll_result_t linked_list_insert_at(linked_list_t l, size_t index, void *data);

ll_result_t   linked_list_reverse  (linked_list_t l);
linked_list_t linked_list_copy     (linked_list_t l);   /* returns NULL on failure */
linked_list_t linked_list_filter   (linked_list_t l, bool (*predicate)(void *, void *), void *context);
ll_result_t   linked_list_remove_if(linked_list_t l, bool (*predicate)(void *, void *), void *context);

#endif /* _LINKED_LIST_H_ */
