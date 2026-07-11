#include <list.h>
#include <pool.h>
#include <allocator.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static void *pool_alloc_fn(void *ctx, size_t size, size_t alignment);

static int pool_resize_fn(void *ctx, void *ptr,
                          size_t old_size, size_t new_size, size_t alignment);

static void *pool_remap_fn(void *ctx, void *ptr,
                           size_t old_size, size_t new_size, size_t alignment);

static void pool_free_fn(void *ctx, void *ptr, size_t size, size_t alignment);

extern const allocator_vtable_t pool_vtable;

linked_list_t pool_list_create(long cap);

void pool_list_free(linked_list_t l);
