#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

typedef struct allocator_vtable {
    void *(*alloc)  (void *ctx, size_t size, size_t alignment);
    int   (*resize) (void *ctx, void *ptr, size_t old_size, size_t new_size, size_t alignment);
    void *(*remap)  (void *ctx, void *ptr, size_t old_size, size_t new_size, size_t alignment);
    void  (*free)   (void *ctx, void *ptr, size_t size, size_t alignment);
} allocator_vtable_t;

typedef struct allocator {
    void                     *ctx;
    const allocator_vtable_t *vtable;
} allocator_t;

void *allocator_alloc  (allocator_t a, size_t size, size_t alignment);
int   allocator_resize (allocator_t a, void *ptr, size_t old_size, size_t new_size, size_t alignment);
void *allocator_remap  (allocator_t a, void *ptr, size_t old_size, size_t new_size, size_t alignment);
void  allocator_free   (allocator_t a, void *ptr, size_t size, size_t alignment);

#endif /* ALLOCATOR_H */
