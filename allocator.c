#include "allocator.h"
#include <stdint.h>
#include <string.h>

/* ── dispatch wrappers ────────────────────────────────────────────────────── */

void *allocator_alloc(allocator_t a, size_t size, size_t alignment)
{
    return a.vtable->alloc(a.ctx, size, alignment);
}

int allocator_resize(allocator_t a, void *ptr, size_t old_size, size_t new_size, size_t alignment)
{
    return a.vtable->resize(a.ctx, ptr, old_size, new_size, alignment);
}

void *allocator_remap(allocator_t a, void *ptr, size_t old_size, size_t new_size, size_t alignment)
{
    return a.vtable->remap(a.ctx, ptr, old_size, new_size, alignment);
}

void allocator_free(allocator_t a, void *ptr, size_t size, size_t alignment)
{
    a.vtable->free(a.ctx, ptr, size, alignment);
}
