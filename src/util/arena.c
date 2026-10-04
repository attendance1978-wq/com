#include "util/arena.h"
#include "util/error.h"

#include <stdlib.h>

#define ARENA_BLOCK_SIZE (16 * 1024)

struct ArenaBlock {
    ArenaBlock *next;
    size_t used;
    size_t cap;
    unsigned char data[];
};

void arena_init(Arena *a) { a->head = NULL; }

void *arena_alloc(Arena *a, size_t size) {
    size = (size + 7u) & ~(size_t)7u;
    if (!a->head || a->head->used + size > a->head->cap) {
        size_t cap = size > ARENA_BLOCK_SIZE ? size : ARENA_BLOCK_SIZE;
        ArenaBlock *b = calloc(1, sizeof(ArenaBlock) + cap);
        if (!b) error_fatal("out of memory");
        b->cap = cap;
        b->next = a->head;
        a->head = b;
    }
    void *p = a->head->data + a->head->used;
    a->head->used += size;
    return p;
}

void arena_free(Arena *a) {
    ArenaBlock *b = a->head;
    while (b) {
        ArenaBlock *n = b->next;
        free(b);
        b = n;
    }
    a->head = NULL;
}
