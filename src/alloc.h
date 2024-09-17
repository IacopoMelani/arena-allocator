#ifndef _ALLOC_H
#define _ALLOC_H

#include <stdlib.h>

/**
 * @struct Allocator
 * @brief A structure that defines a custom memory allocator.
 *
 * This structure provides function pointers for custom memory allocation,
 * deallocation, reallocation, and zero-initialized allocation. The context field
 * is actually a pointer to the effective allocator, which can be a custom allocator.
 *
 * @see https://bytesbeneath.com/articles/the-arena-custom-memory-allocators based on works of Dylan Falconer.
 */
typedef struct {
    /**
     * @brief Allocates memory.
     *
     * @param size The size of the memory block to allocate.
     * @param context A pointer to the effective allocator.
     *
     */
    void *(*alloc)(size_t size, void *context);
    /**
     * @brief Frees memory.
     *
     * @param size The size of the memory block to free.
     * @param ptr A pointer to the memory block to free.
     * @param context A pointer to the effective allocator.
     */
    void (*free)(size_t size, void *ptr, void *context);
    /**
     * @brief Resizes the memory block.
     *
     * @param new_size The new size of the memory block.
     * @param old_size The old size of the memory block.
     * @param ptr A pointer to the memory block to resize.
     * @param context A pointer to the effective allocator.
     * @return A pointer to the resized memory block.
     */
    void *(*realloc)(size_t new_size, size_t old_size, void *ptr, void *context);
    /**
     * @brief Allocates memory and initializes it to zero.
     *
     * @param count The number of elements to allocate.
     * @param size The size of the elements.
     * @param context A pointer to the effective allocator.
     * @return A pointer to the allocated memory.
     */
    void *(*calloc)(size_t count, size_t size, void *context);
    /**
     * @brief A function pointer that returns the total allocated memory actually used, this can be used for debugging
     * to check for memory leaks.
     * Allocator implements their own version of this function but should return the total allocated memory so user
     * requested + alignment.
     *
     * @param context A pointer to the effective allocator.
     * @return The total allocated memory.
     */
    size_t (*allocated)(void *context);
    /**
     * @brief A pointer to the effective allocator.
     */
    void *context;
} Allocator;

/**
 * @brief Allocates memory for n elements of type T.
 *
 * @param T The type of the elements.
 * @param n The number of elements to allocate.
 * @param a The Allocator structure.
 * @return A pointer to the allocated memory.
 */
#define make(T, n, a) ((T *)((a).alloc(sizeof(T) * n, a.context)))
/**
 * @brief Resizes the memory block pointed to by p to size s.
 *
 * @param T The type of the elements.
 * @param s The new size of the memory block.
 * @param o The old size of the memory block.
 * @param p The pointer to the memory block.
 * @param a The Allocator structure.
 * @return A pointer to the resized memory block.
 */
#define resize(T, s, o, p, a) ((T *)((a).realloc(sizeof(T) * s, sizeof(T) * o, p, a.context)))
/**
 * @brief Allocates memory for n elements of type T and initializes it to zero.
 *
 * @param T The type of the elements.
 * @param n The number of elements to allocate.
 * @param a The Allocator structure.
 * @return A pointer to the allocated memory.
 */
#define make_zeroed(T, n, a) ((T *)((a).calloc(n, sizeof(T), a.context)))
/**
 * @brief Frees the memory block pointed to by p.
 *
 * @param T The type of the elements.
 * @param s The size of the memory block.
 * @param p The pointer to the memory block.
 * @param a The Allocator structure.
 */
#define release(T, s, p, a) ((a).free(sizeof(T) * s, p, a.context))
/**
 * @brief Returns the total allocated memory, this can be used for debugging to check for memory leaks.
 *
 * @param a The Allocator structure.
 * @return The total allocated memory.
 */
#define allocated(a) ((a).allocated(a.context))

#endif // _ALLOC_H
