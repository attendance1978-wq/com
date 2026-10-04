#ifndef COM_UTIL_ARENA_H
#define COM_UTIL_ARENA_H

#include <stddef.h>

typedef struct ArenaBlock ArenaBlock;
typedef struct {
    ArenaBlock *head;
} Arena;

void arena_init(Arena *a);
/* Returns zero-initialised memory, valid until arena_free. */
void *arena_alloc(Arena *a, size_t size);
void arena_free(Arena *a);

#endif
