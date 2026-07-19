

#include "pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

int n = 12;
void* ctx;
size_t sz;
pool_t *pool;

void test_alloc() {
  // Allocate data and metadata
  ctx = mmap(NULL,
      sz,
      PROT_READ | PROT_WRITE,
      MAP_PRIVATE | MAP_ANONYMOUS,
      -1,
      0);

  if (ctx == MAP_FAILED) {
    fprintf(stderr, "mmap fail\n");
    exit(1);
  }

  pool = pool_create(ctx, n * (sizeof(int) + sizeof(pool_t)), sizeof(int));
}


int main() {
  sz = n * (sizeof(int) + sizeof(pool_t));
  test_alloc();

  for (int i = 0; i < n -1; i++) {
    printf("%x\n", (size_t)pool_alloc(pool));
  }

  munmap(ctx, sz);
  return 0;
}
