// pool.c

#include "pool.h"
#include <stddef.h>

// simply works as adding an item to a stack
void __pool_push(pool_t *self, pool_t *node)
{
  node->next = self->next;
  self->next = node;
}

// pool_t *pool_create_from_mmap(uint32_t size, uint32_t object_size) {
//   void* ctx = mmap(NULL,
//       size,
//       PROT_READ | PROT_WRITE,
//       MAP_PRIVATE | MAP_ANONYMOUS,
//       -1,
//       0);
// 
//   if (ctx == MAP_FAILED) {
//     fprintf(stderr, "mmap fail\n");
//     exit(1);
//   }
// 
//   return pool_create(ctx, size, object_size);
// }

// create a pool using 
// 'size' needs to account for metadata, hence size = n * (object_size + sizeof(pool_t))
pool_t *pool_create(void *memory_space, uint32_t size, uint32_t object_size)
{
  // initialize the first bytes to the pool instance itself
  pool_t *self = (pool_t *)(memory_space);
  self->next = NULL;

  // get the address of the memory space
  size_t address = (size_t)memory_space;

  // initialize a cursor to calculate chunk addresses
  size_t cursor = sizeof(pool_t);

  // actual chunk size, which is the
  // block(object) size + the size of its header
  size_t chunk_size = object_size + sizeof(pool_t);

  while (1)
  {
    // if the next chunk cannot fit into the memory space
    // break the loop, that's enough partitioning for today!
    if (cursor + chunk_size > size)
      break;

    // extend the pool chunks with current memory address
    __pool_push(self, (pool_t *)(address + cursor));

    // move the cursor
    cursor += chunk_size;
  }
  return self;
}

// just like you pop an item from a stack
// pool pop is a helper function that will do that for you
pool_t *__pool_pop(pool_t *self)
{
  pool_t *node = self->next;
  if (!node)
    return NULL;
  self->next = node->next;
  return node;
}


// allocate memory and return the reference to the user
void *pool_alloc(pool_t *self)
{
  // pop a chunk from the stack
  pool_t *node = __pool_pop(self);
  // if it's null, we're probably out of memory!
  if (!node)
    return NULL;

  // return a valid reference to the data section
  return (void *)((size_t)node + sizeof(pool_t));
}

// free the allocated memory and return it back to the pool
void pool_free(pool_t *self, void *ptr)
{
  if (!ptr)
    return;
  // as you remember, we did shift the pointer when alloc function is called
  // so we need to shift it back, to access the pool header
  pool_t *node = (pool_t *)(ptr - sizeof(pool_t));
  // extend the pool with this new available
  __pool_push(self, node);
}
