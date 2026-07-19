/*
 * test_list_pool.c
 *
 * Exercises linked_list using a pool allocator whose backing store is mmap'd
 * memory.  Compile from the directory containing all headers + their .c impls:
 *
 *   gcc -std=c11 -Wall -Wextra -I. test_list_pool.c \
 *       list.c pool.c allocator.c -o test_list_pool
 *
 * Prints PASS / FAIL per case, exits 0 iff all pass.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>

#include "allocator.h"
#include "pool.h"
#include "list.h"

/* -------------------------------------------------------------------------
 * Test scaffolding
 * ---------------------------------------------------------------------- */

static int g_failures = 0;

#define CHECK(cond, label)                                               \
    do {                                                                 \
        if (cond) {                                                      \
            printf("  PASS  %s\n", (label));                            \
        } else {                                                         \
            printf("  FAIL  %s  (line %d)\n", (label), __LINE__);      \
            g_failures++;                                                \
        }                                                                \
    } while (0)

/* -------------------------------------------------------------------------
 * Pool -> allocator_t bridge
 *
 * pool_alloc/pool_free don't take size/alignment; we ignore those params.
 * allocator_t.ctx points to the pool_t (which lives inside the mmap region).
 * ---------------------------------------------------------------------- */

static void *pool_alloc_fn(void *ctx, size_t size, size_t alignment)
{
    (void)size; (void)alignment;
    return pool_alloc((pool_t *)ctx);
}

static int pool_resize_fn(void *ctx, void *ptr,
                          size_t old_size, size_t new_size, size_t alignment)
{
    (void)ctx; (void)ptr; (void)alignment;
    return (old_size == new_size) ? 0 : -1;
}

static void *pool_remap_fn(void *ctx, void *ptr,
                           size_t old_size, size_t new_size, size_t alignment)
{
    (void)ctx; (void)ptr; (void)old_size; (void)new_size; (void)alignment;
    return NULL;
}

static void pool_free_fn(void *ctx, void *ptr, size_t size, size_t alignment)
{
    (void)size; (void)alignment;
    pool_free((pool_t *)ctx, ptr);
}

static const allocator_vtable_t pool_vtable = {
    .alloc  = pool_alloc_fn,
    .resize = pool_resize_fn,
    .remap  = pool_remap_fn,
    .free   = pool_free_fn,
};

/* mmap a region, init a pool inside it, return a ready allocator_t. */
static allocator_t make_pool_allocator(pool_t **out_pool,
                                       void   **out_mmap,
                                       size_t   mmap_size,
                                       uint32_t object_size)
{
    void *mem = mmap(NULL, mmap_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1, 0);
    if (mem == MAP_FAILED) { perror("mmap"); exit(EXIT_FAILURE); }

    pool_t *pool = pool_create(mem, (uint32_t)mmap_size, object_size);
    if (!pool) {
        fprintf(stderr, "pool_create returned NULL\n");
        munmap(mem, mmap_size);
        exit(EXIT_FAILURE);
    }

    *out_pool = pool;
    *out_mmap = mem;
    return (allocator_t){ .ctx = pool, .vtable = &pool_vtable };
}

/* -------------------------------------------------------------------------
 * Callback helpers
 * ---------------------------------------------------------------------- */

static void print_int(void *data, void *ctx)
{
    (void)ctx;
    printf("    %d\n", *(int *)data);
}

static void *double_int(void *data, void *ctx)
{
    (void)ctx;
    *(int *)data *= 2;
    return data;
}

static bool is_even(void *data, void *ctx)
{
    (void)ctx;
    return (*(int *)data % 2) == 0;
}

/* -------------------------------------------------------------------------
 * Test cases
 * ---------------------------------------------------------------------- */

static void test_push_pop(allocator_t alloc)
{
    printf("\n[push / pop]\n");

    linked_list_t l = linked_list_init(alloc);
    CHECK(l != NULL, "init");

    int vals[5] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++)
        linked_list_push_back(l, &vals[i]);

    CHECK(linked_list_size(l) == 5, "size == 5 after 5x push_back");

    ll_maybe_t r = linked_list_pop_front(l);
    CHECK(r.err == LL_OK && *(int *)r.value == 10, "pop_front == 10");

    r = linked_list_pop_back(l);
    CHECK(r.err == LL_OK && *(int *)r.value == 50, "pop_back == 50");

    CHECK(linked_list_size(l) == 3, "size == 3 after 2 pops");

    linked_list_free(l);
}

static void test_push_front(allocator_t alloc)
{
    printf("\n[push_front]\n");

    linked_list_t l = linked_list_init(alloc);
    int a = 1, b = 2, c = 3;
    linked_list_push_front(l, &a);
    linked_list_push_front(l, &b);
    linked_list_push_front(l, &c);   /* [3, 2, 1] */

    ll_maybe_t r = linked_list_get_at(l, 0);
    CHECK(r.err == LL_OK && *(int *)r.value == 3, "head == 3");

    r = linked_list_get_at(l, 2);
    CHECK(r.err == LL_OK && *(int *)r.value == 1, "tail == 1");

    linked_list_free(l);
}

static void test_insert_remove(allocator_t alloc)
{
    printf("\n[insert_at / remove_at / get_at]\n");

    linked_list_t l = linked_list_init(alloc);
    int a = 1, b = 2, c = 3;
    linked_list_push_back(l, &a);
    linked_list_push_back(l, &b);
    linked_list_push_back(l, &c);        /* [1, 2, 3] */

    int x = 99;
    ll_result_t res = linked_list_insert_at(l, 1, &x); /* [1, 99, 2, 3] */
    CHECK(res.err == LL_OK,                "insert_at(1) ok");
    CHECK(linked_list_size(l) == 4,        "size == 4 after insert");

    ll_maybe_t got = linked_list_get_at(l, 1);
    CHECK(got.err == LL_OK && *(int *)got.value == 99, "get_at(1) == 99");

    res = linked_list_remove_at(l, 1);   /* [1, 2, 3] */
    CHECK(res.err == LL_OK,                "remove_at(1) ok");

    got = linked_list_get_at(l, 1);
    CHECK(got.err == LL_OK && *(int *)got.value == 2, "get_at(1) == 2 after remove");

    got = linked_list_get_at(l, 10);
    CHECK(got.err == LL_ERR_OUT_OF_BOUNDS, "get_at(10) -> OUT_OF_BOUNDS");

    res = linked_list_insert_at(l, 100, &x);
    CHECK(res.err == LL_ERR_OUT_OF_BOUNDS, "insert_at(100) -> OUT_OF_BOUNDS");

    linked_list_free(l);
}

static void test_for_each_map(allocator_t alloc)
{
    printf("\n[for_each / map]\n");

    linked_list_t l = linked_list_init(alloc);
    int vals[4] = {1, 2, 3, 4};
    for (int i = 0; i < 4; i++)
        linked_list_push_back(l, &vals[i]);

    printf("  before map:\n");
    linked_list_for_each(l, print_int, NULL);

    linked_list_map(l, double_int, NULL);

    printf("  after doubling:\n");
    linked_list_for_each(l, print_int, NULL);

    ll_maybe_t r = linked_list_get_at(l, 0);
    CHECK(r.err == LL_OK && *(int *)r.value == 2, "map: vals[0] == 2");
    r = linked_list_get_at(l, 3);
    CHECK(r.err == LL_OK && *(int *)r.value == 8, "map: vals[3] == 8");

    linked_list_free(l);
}

static void test_filter_remove_if(allocator_t alloc)
{
    printf("\n[filter / remove_if]\n");

    linked_list_t l = linked_list_init(alloc);
    int vals[6] = {1, 2, 3, 4, 5, 6};
    for (int i = 0; i < 6; i++)
        linked_list_push_back(l, &vals[i]);

    linked_list_t evens = linked_list_filter(l, is_even, NULL);
    CHECK(evens != NULL,                  "filter non-NULL");
    CHECK(linked_list_size(evens) == 3,   "filter: 3 even numbers");

    ll_result_t res = linked_list_remove_if(l, is_even, NULL);
    CHECK(res.err == LL_OK,               "remove_if ok");
    CHECK(linked_list_size(l) == 3,       "remove_if: 3 odds remain");

    ll_maybe_t r = linked_list_get_at(l, 0);
    CHECK(r.err == LL_OK && *(int *)r.value == 1, "first odd == 1");
    r = linked_list_get_at(l, 2);
    CHECK(r.err == LL_OK && *(int *)r.value == 5, "last odd == 5");

    linked_list_free(evens);
    linked_list_free(l);
}

static void test_reverse_copy(allocator_t alloc)
{
    printf("\n[reverse / copy]\n");

    linked_list_t l = linked_list_init(alloc);
    int vals[4] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++)
        linked_list_push_back(l, &vals[i]);

    linked_list_t cp = linked_list_copy(l);
    CHECK(cp != NULL,                     "copy non-NULL");
    CHECK(linked_list_size(cp) == 4,      "copy size == 4");

    linked_list_reverse(l);

    ll_maybe_t r = linked_list_get_at(l, 0);
    CHECK(r.err == LL_OK && *(int *)r.value == 40, "reverse: head == 40");
    r = linked_list_get_at(l, 3);
    CHECK(r.err == LL_OK && *(int *)r.value == 10, "reverse: tail == 10");

    r = linked_list_get_at(cp, 0);
    CHECK(r.err == LL_OK && *(int *)r.value == 10, "copy head still == 10");

    linked_list_free(cp);
    linked_list_free(l);
}

static void test_clear_empty(allocator_t alloc)
{
    printf("\n[clear / is_empty]\n");

    linked_list_t l = linked_list_init(alloc);
    CHECK(linked_list_is_empty(l),        "new list is_empty");

    int v = 7;
    linked_list_push_back(l, &v);
    CHECK(!linked_list_is_empty(l),       "non-empty after push");

    linked_list_clear(l);
    CHECK(linked_list_is_empty(l),        "empty after clear");
    CHECK(linked_list_size(l) == 0,       "size == 0 after clear");

    linked_list_free(l);
}

static void test_error_paths(allocator_t alloc)
{
    printf("\n[error paths]\n");

    ll_result_t r = linked_list_push_back(NULL, NULL);
    CHECK(r.err == LL_ERR_NULL_LIST,      "push_back(NULL) -> NULL_LIST");

    ll_maybe_t m = linked_list_pop_back(NULL);
    CHECK(m.err == LL_ERR_NULL_LIST,      "pop_back(NULL) -> NULL_LIST");

    linked_list_t l = linked_list_init(alloc);

    r = linked_list_for_each(l, NULL, NULL);
    CHECK(r.err == LL_ERR_NULL_FUNC,      "for_each(NULL cb) -> NULL_FUNC");

    r = linked_list_map(l, NULL, NULL);
    CHECK(r.err == LL_ERR_NULL_FUNC,      "map(NULL cb) -> NULL_FUNC");

    m = linked_list_pop_front(l);
    CHECK(m.err == LL_ERR_OUT_OF_BOUNDS,  "pop_front empty -> OUT_OF_BOUNDS");

    m = linked_list_pop_back(l);
    CHECK(m.err == LL_ERR_OUT_OF_BOUNDS,  "pop_back empty -> OUT_OF_BOUNDS");

    linked_list_free(l);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */

int main(void)
{
    /*
     * Each linked_list_node is the fixed object size for the pool.
     * 128 slots is generous; peak live node count across all tests is ~20.
     * Add a 4 KiB page of headroom for pool_create's own bookkeeping.
     */
    const size_t   NODE_SIZE  = sizeof(linked_list_node);
    const uint32_t POOL_SLOTS = 128;
    const size_t   MMAP_SIZE  = NODE_SIZE * POOL_SLOTS + 4096;

    pool_t *pool   = NULL;
    void   *mapped = NULL;

    allocator_t alloc = make_pool_allocator(&pool, &mapped,
                                            MMAP_SIZE, (uint32_t)NODE_SIZE);

    printf("=== linked_list + pool allocator (mmap-backed) ===\n");
    printf("    sizeof(linked_list_node) : %zu\n", NODE_SIZE);
    printf("    pool slots               : %u\n",  POOL_SLOTS);
    printf("    mmap region              : %zu bytes\n", MMAP_SIZE);

    test_push_pop        (alloc);
    test_push_front      (alloc);
    test_insert_remove   (alloc);
    test_for_each_map    (alloc);
    test_filter_remove_if(alloc);
    test_reverse_copy    (alloc);
    test_clear_empty     (alloc);
    test_error_paths     (alloc);

    munmap(mapped, MMAP_SIZE);

    printf("\n=== %s (%d failure(s)) ===\n",
           g_failures == 0 ? "ALL PASS" : "SOME FAILED",
           g_failures);

    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
