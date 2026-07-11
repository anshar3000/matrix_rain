#ifndef POOL_H
#define POOL_H

#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>

typedef struct pool_t {
    struct pool_t *next;
} pool_t;


// pool_t *pool_create_from_mmap(uint32_t size, uint32_t object_size);
pool_t *pool_create(void *memory_space, uint32_t size, uint32_t object_size);
void   *pool_alloc(pool_t *self);
void    pool_free(pool_t *self, void *ptr);

#endif /* POOL_H */
