#include "arena.h"
#include "utils.h"

#include <malloc/_malloc.h>
#include <memory.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#define printf(...) printf(__VA_ARGS__)
#else
#define printf(x, ...)
#endif

/**
 * @brief Defines the available free list classes for reusing blocks
 */
typedef enum {
    Small = 0,  // 0 - 64 bytes
    Medium = 1, // 64 - 512 bytes
    Large = 2,  // 512 - 4096 bytes
    Huge = 3,   // 4096 - the rest
} BlockClass;

/**
 * @brief Allocate memory from the arena without locking or other high-level operations
 *
 * @param size size of the memory to allocate
 * @param a arena to allocate from
 * @return void* pointer to the allocated memory
 */
static void *arena_internal_alloc(size_t size, Arena *arena);
/**
 * @brief Allocate memory from the arena with alignment specified by the arena
 *
 * @param a arena to allocate from
 * @param size size of the memory to allocate
 * @return void* pointer to the allocated memory
 */
static void *arena_alloc_aligned(Arena *a, size_t size);
/**
 * @brief Align a pointer to the specified alignment
 *
 * @param ptr pointer to align
 * @param alignment alignment to align to
 * @return uintptr_t aligned pointer
 */
static uintptr_t align_forward(uintptr_t ptr, size_t alignment);
/**
 * @brief Recycle a block of memory back into the appropriate free list
 *
 * @param a arena to recycle the block into
 * @param ptr pointer to the block
 * @param size size of the block
 * @return size_t size of the block
 */
static size_t arena_recycle_alloc(Arena *a, void *ptr, size_t size, BlockClass class);
/**
 * @brief Find the first block that fits the requested size in the free list
 *
 * @param a arena to search in
 * @param size size of the block
 * @return void* pointer to the block
 */
static void *arena_free_list_find_first_block(Arena *a, BlockClass class, size_t size);
/**
 * @brief Find the best block that fits the requested size in the free list
 *
 * @param a arena to search in
 * @param size size of the block
 * @return void* pointer to the block
 */
static void *arena_free_list_find_best_block(Arena *a, BlockClass class, size_t size);
/**
 * @brief Get the block class object
 *
 * @param size size of the block
 * @return BlockClass most appropriate block class
 */
static inline BlockClass get_block_class(size_t size);

Arena arena_init(void *buffer, size_t size, size_t align, AllocationStrategy strategy) {
    return (Arena){
        .base = buffer,
        .size = size,
        .align = align,
        .offset = 0,
        .committed = 0,
        .free_list = {0},
        .strategy = strategy,
        .mutex = NULL,
    };
}

void *arena_alloc(size_t size, void *context) {
    Arena *a = (Arena *)context;
    arena_lock(a);
    void *ptr = arena_internal_alloc(size, a);
    arena_unlock(a);
    return ptr;
}

void *arena_realloc(size_t new_size, size_t old_size, void *ptr, void *context) {
    if (new_size <= old_size) {
        return ptr;
    }

    Arena *a = (Arena *)context;
    arena_lock(a);

    void *new_ptr = arena_internal_alloc(new_size, a);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size);
    }
    BlockClass class = get_block_class(old_size);
    arena_recycle_alloc((Arena *)context, ptr, old_size, class);
    arena_unlock(a);
    return new_ptr;
}

void *arena_calloc(size_t count, size_t size, void *context) {
    Arena *a = (Arena *)context;
    arena_lock(a);
    size_t total_size = count * size;
    void *ptr = arena_internal_alloc(total_size, a);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    arena_unlock(a);
    return ptr;
}

void arena_free(size_t size, void *ptr, void *context) {
    BlockClass class = get_block_class(size);
    Arena *a = (Arena *)context;
    arena_lock(a);
    arena_recycle_alloc((Arena *)context, ptr, size, class);
    arena_unlock(a);
}

void arena_free_all(void *context) {
    Arena *a = (Arena *)context;
    arena_lock(a);
    a->offset = 0;
    a->committed = 0;
    for (int i = 0; i < FREE_LIST_CLASSES; i++) {
        a->free_list[i] = 0;
    }
    arena_unlock(a);
}

void arena_lock(Arena *a) {
    if (!a->mutex) {
        return;
    }
    int ret = pthread_mutex_lock(a->mutex);
    if (ret != 0) {
        error("Failed to lock mutex\n");
        exit(1);
    }
}

void arena_unlock(Arena *a) {
    if (!a->mutex) {
        return;
    }
    int ret = pthread_mutex_unlock(a->mutex);
    if (ret != 0) {
        error("Failed to unlock mutex\n");
        exit(1);
    }
}

static void *arena_alloc_aligned(Arena *a, size_t size) {
    uintptr_t curr_ptr = (uintptr_t)a->base + (uintptr_t)a->offset;
    uintptr_t offset = align_forward(curr_ptr, a->align);
    offset -= (uintptr_t)a->base;

    if (offset + size > a->size) {
        return 0;
    }

    a->committed += size;
    void *ptr = (uint8_t *)a->base + offset;
    a->offset = offset + size;

    return ptr;
}

static void *arena_internal_alloc(size_t size, Arena *a) {
    if (!size) {
        return 0;
    }

    void *ptr = 0;
    BlockClass class = get_block_class(size);

    if (a->strategy == FirstFit) {
        ptr = arena_free_list_find_first_block(a, class, size);
    } else if (a->strategy == BestFit) {
        ptr = arena_free_list_find_best_block(a, class, size);
    }
    if (ptr) {
        printf("------\n");
        printf("Reusing ptr: %zu\n", (uintptr_t)ptr);
        printf("Reusing size: %zu\n", size);
        printf("------\n");
        a->committed += size;
        return ptr;
    }
    void *alloc = arena_alloc_aligned(a, size);

    printf("------\n");
    printf("Allocating ptr: %zu\n", (uintptr_t)alloc);
    printf("Allocating size: %zu\n", size);
    printf("------\n");

    return alloc;
}

static size_t arena_recycle_alloc(Arena *a, void *ptr, size_t size, BlockClass class) {
    if (size < sizeof(Block)) {
        return 0;
    }

    Block *block = (Block *)ptr;

    uintptr_t cons_block = align_forward((uintptr_t)ptr + size, a->align);
    size_t pad = (size_t)(cons_block - ((uintptr_t)ptr + size));

    block->size = size + pad;
    block->next = a->free_list[class];
    a->free_list[class] = block;
    a->committed -= size;

    printf("------\n");
    printf("Freeing ptr: %zu\n", (uintptr_t)ptr);
    printf("Freeing cons_block: %zu\n", cons_block);
    printf("Freeing size: %zu\n", size);
    printf("Freeing pad: %zu\n", pad);
    printf("freeing block of size %zu\n", block->size);
    printf("------\n");

    return block->size;
}

static void *arena_free_list_find_first_block(Arena *a, BlockClass class, size_t size) {
    if (!a->free_list[class]) {
        return 0;
    }

    Block *prev = 0;
    Block *curr = a->free_list[class];

    while (curr) {
        if (curr->size >= size) {
            if (prev) {
                prev->next = curr->next;
            } else {
                a->free_list[class] = curr->next;
            }
            return (void *)curr;
        }

        prev = curr;
        curr = curr->next;
    }

    return 0;
}

static void *arena_free_list_find_best_block(Arena *a, BlockClass class, size_t size) {
    if (!a->free_list[class]) {
        return 0;
    }

    Block *prev = 0;
    Block *curr = a->free_list[class];
    Block *best = 0;

    while (curr) {
        if (curr->size >= size) {
            if (!best || (best && curr->size < best->size)) {
                best = curr;
                if (best->size == size) {
                    break;
                }
            }
        }

        if (!curr->next) {
            break;
        }

        prev = curr;
        curr = curr->next;
    }

    if (best) {
        if (prev) {
            prev->next = best->next;
        } else {
            a->free_list[class] = best->next;
        }

        return (void *)best;
    }

    return 0;
}

static uintptr_t align_forward(uintptr_t ptr, size_t alignment) {
    uintptr_t p, a, modulo;

    if (!is_power_of_two(alignment)) {
        return 0;
    }

    p = ptr;
    a = (uintptr_t)alignment;
    modulo = p & (a - 1);

    if (modulo) {
        p += a - modulo;
    }

    return p;
}

static inline BlockClass get_block_class(size_t size) {
    if (size <= 64) {
        return Small;
    } else if (size <= 512) {
        return Medium;
    } else if (size <= 4096) {
        return Large;
    } else {
        return Huge;
    }
}
