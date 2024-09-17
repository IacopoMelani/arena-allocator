#ifndef _ARENA_H
#define _ARENA_H

#include "alloc.h"
#include <pthread.h>

// Number of free list classes
#define FREE_LIST_CLASSES 4

// Default memory alignment
#define DEFAULT_ALLIGNMENT (2 * sizeof(void *)) // 16 bytes

/**
 * @brief Block structure for the free list
 *
 * @param size size of the block
 * @param next pointer to the next block
 */

typedef struct Block {
    size_t size;
    struct Block *next;
} Block;

/**
 * @brief Allocation strategy for reusing blocks
 *
 * BestFit: Find the smallest block that fits the requested size
 *
 * FirstFit: Find the first block that fits the requested size
 */
typedef enum {
    BestFit = 0,
    FirstFit = 1,
} AllocationStrategy;

/**
 * @brief Arena structure for memory allocation
 *
 * @param base base address of the arena
 * @param align memory alignment of each block
 * @param size size of the arena
 * @param offset current offset in the arena
 * @param committed amount of memory committed in the arena
 * @param free_list list of freed blocks and reusables, divided into classes
 * @param strategy allocation strategy for reusing blocks
 */
typedef struct {
    void *base;
    size_t align;
    size_t size;
    size_t offset;
    size_t committed;
    Block *free_list[FREE_LIST_CLASSES];
    AllocationStrategy strategy;
    // mutex
    pthread_mutex_t *mutex;
} Arena;

/**
 * @brief Initialize an allocator with an arena
 */
#define arena_alloc_init(a)                                                                                            \
    (Allocator) { arena_alloc, arena_free, arena_realloc, arena_calloc, arena_allocated, a }

/**
 * @brief Initialize an arena with a buffer, size, alignment and allocation strategy
 *
 * @param buffer buffer to use for the arena
 * @param size size of the buffer
 * @param align alignment of the buffer, must be a power of 2, use DEFAULT_ALLIGNMENT for default
 * @param strategy strategy for reusing blocks
 * @return Arena
 */
Arena arena_init(void *buffer, size_t size, size_t align, AllocationStrategy strategy);
/**
 * @brief Allocate memory from the arena
 *
 * @param size size of the memory to allocate
 * @param context arena to allocate from, is a void* to statify the Allocator interface
 * @return void* pointer to the allocated memory
 */
void *arena_alloc(size_t size, void *context);
/**
 * @brief Reallocate memory from the arena
 *
 * @param new_size new size of the memory to allocate
 * @param old_size old size of the memory to reallocate
 * @param ptr pointer to the memory to reallocate
 * @param context arena to reallocate from, is a void* to statify the Allocator interface
 * @return void* pointer to the reallocated memory
 */
void *arena_realloc(size_t new_size, size_t old_size, void *ptr, void *context);
/**
 * @brief Allocate memory from the arena and set it to zero
 *
 * @param count number of elements to allocate
 * @param size size of each element
 * @param context arena to allocate from, is a void* to statify the Allocator interface
 * @return void* pointer to the allocated memory
 */
void *arena_calloc(size_t count, size_t size, void *context);
/**
 * @brief Free memory from the arena
 *
 * @param size size of the memory to free
 * @param ptr pointer to the memory to free
 * @param context arena to free from, is a void* to statify the Allocator interface
 */
void arena_free(size_t size, void *ptr, void *context);
/**
 * @brief Free all memory from the arena
 *
 * @param context arena to free from, is a void* to statify the Allocator interface
 */
void arena_free_all(void *context);
/**
 * @brief Lock the arena mutex, if it exists
 *
 * @param a arena to lock
 */
void arena_lock(Arena *a);
/**
 * @brief Unlock the arena mutex, if it exists
 *
 * @param a arena to unlock
 */
void arena_unlock(Arena *a);
/**
 * @brief Get the total allocated memory from the arena
 *
 * @param context arena to get the allocated memory from, is a void* to statify the Allocator interface
 * @return size_t total allocated memory
 */
static inline size_t arena_allocated(void *context) {
    Arena *a = (Arena *)context;
    arena_lock(a);
    size_t comm = a->committed;
    arena_unlock(a);
    return comm;
}

/**
 * @brief Set the mutex for the arena
 *
 * @param a arena to set the mutex for
 * @param mutex mutex to set, mutex should be already allocated and initialized by the user before calling this
 * function, setting a NULL mutex will disable the mutex
 * This function is not thread safe, make sure to not call it while other threads are using the arena.
 */
static inline void arena_set_attr_mutex(Arena *a, pthread_mutex_t *mutex) { a->mutex = mutex; }

#endif // _ARENA_H
