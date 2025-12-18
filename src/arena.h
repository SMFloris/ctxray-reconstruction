#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_BLOCK_SIZE (16 * 1024)

typedef struct ArenaBlock {
  struct ArenaBlock *next;
  size_t used;
  unsigned char data[ARENA_BLOCK_SIZE];
} ArenaBlock;

typedef struct {
  ArenaBlock *head;
} Arena;

static inline Arena *arena_create(void) {
  Arena *a = (Arena *)calloc(1, sizeof(Arena));
  return a;
}

static inline void *arena_alloc(Arena *a, size_t size) {
  // Handle large allocations that exceed block size
  if (size > ARENA_BLOCK_SIZE) {
    size_t block_size = offsetof(ArenaBlock, data) + size;
    ArenaBlock *b = (ArenaBlock *)calloc(1, block_size);
    b->used = size;
    b->next = a->head;
    a->head = b;
    return b->data;
  }

  if (!a->head || a->head->used + size > ARENA_BLOCK_SIZE) {
    ArenaBlock *b = (ArenaBlock *)calloc(1, sizeof(ArenaBlock));
    b->next = a->head;
    a->head = b;
  }

  void *ptr = a->head->data + a->head->used;
  a->head->used += size;
  return ptr;
}

static inline void *arena_alloc_zero(Arena *a, size_t size) {
  void *p = arena_alloc(a, size);
  memset(p, 0, size);
  return p;
}

static inline void arena_reset(Arena *a) {
  for (ArenaBlock *b = a->head; b; b = b->next)
    b->used = 0;
}

static inline void arena_destroy(Arena *a) {
  ArenaBlock *b = a->head;
  while (b) {
    ArenaBlock *n = b->next;
    free(b);
    b = n;
  }
  free(a);
}
