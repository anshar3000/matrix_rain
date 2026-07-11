#include "pool_list.h"

static void *pool_alloc_fn(void *ctx, size_t size, size_t alignment)
{
    (void)size;
    (void)alignment;
    return pool_alloc((pool_t *)ctx);
}

static int pool_resize_fn(void *ctx, void *ptr,
                          size_t old_size, size_t new_size, size_t alignment)
{
    (void)ctx; (void)ptr; (void)old_size; (void)new_size; (void)alignment;
    return -1; /* unsupported */
}

static void *pool_remap_fn(void *ctx, void *ptr,
                           size_t old_size, size_t new_size, size_t alignment)
{
    (void)ctx; (void)ptr; (void)old_size; (void)new_size; (void)alignment;
    return NULL; /* unsupported */
}

static void pool_free_fn(void *ctx, void *ptr, size_t size, size_t alignment)
{
    (void)size;
    (void)alignment;
    pool_free((pool_t *)ctx, ptr);
}

const allocator_vtable_t pool_vtable = {
    .alloc  = pool_alloc_fn,
    .resize = pool_resize_fn,
    .remap  = pool_remap_fn,
    .free   = pool_free_fn,
};

linked_list_t pool_list_create(long cap)
{
    /* Each chunk = pool_t header + node payload.
     * Leading sizeof(pool_t) is consumed by the pool header itself. */
    size_t chunk_size = sizeof(pool_t) + sizeof(linked_list_node);
    size_t buf_size   = sizeof(pool_t) + (size_t)cap * chunk_size;

    void *pool_buf = malloc(buf_size);
    if (!pool_buf) {
        fprintf(stderr, "pool_list_create -> malloc fail\n");
        return NULL;
    }

    pool_t *pool = pool_create(pool_buf, (uint32_t)buf_size,
                               sizeof(linked_list_node));

    allocator_t alloc = { .ctx = pool, .vtable = &pool_vtable };

    linked_list_t l = linked_list_init(alloc);
    if (!l) {
        fprintf(stderr, "pool_list_create -> linked_list_init failed\n");
        return NULL;
    }

    return l;
}

void pool_list_free(linked_list_t l) 
{
   if(l == NULL) return;
   void* ctx= l->allocator.ctx;
   
   if(ctx) free(ctx);
   linked_list_free(l);
}
