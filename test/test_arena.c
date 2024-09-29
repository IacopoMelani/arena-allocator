#include "../src/arena.h"
#include "../src/memdump.h"
#include "../src/utils.h"

typedef struct {
    size_t y;
    int x;
    char z;
} Data;

int main(void) {

    size_t size = 1024 * 1024 * 64;

    void *buffer = malloc(size);
    Arena arena = arena_init(buffer, size, DEFAULT_ALLIGNMENT, BestFit);
    Allocator allocator = arena_alloc_init(&arena);

    int *x = make(int, 420, allocator);
    size_t *y = make(size_t, 23, allocator);
    char *z = make(char, 69, allocator);

    for (int i = 0; i < 420; i += 1)
        x[i] = 97; // 'a'

    for (int i = 0; i < 23; i += 1)
        y[i] = (size_t)98; // 'b'

    for (int i = 0; i < 69; i += 1)
        z[i] = 99; // 'c'

    z = resize(char, 100, 69, z, allocator);
    for (int i = 69; i < 100; i += 1)
        z[i] = 99; // 'c'

    int *zerod = make_zeroed(int, 100, allocator);

    for (int i = 0; i < 100; i += 1)
        zerod[i] = 100; // 'd'

    hexDump("buffer", buffer, arena.offset);
    release(int, 420, x, allocator);
    release(size_t, 23, y, allocator);

    size_t *new_y = make(size_t, 24, allocator);
    for (int i = 0; i < 24; i += 1)
        new_y[i] = (size_t)101; // 'e'

    char *new_z = make(char, 78, allocator);
    for (int i = 0; i < 78; i += 1)
        new_z[i] = 102; // 'f'

    int *new_x = make_zeroed(int, 400, allocator);
    for (int i = 0; i < 400; i += 1)
        new_x[i] = 103; // 'g'

    release(int, 100, zerod, allocator);

    int *new_zerod = make_zeroed(int, 92, allocator);
    for (int i = 0; i < 92; i += 1)
        new_zerod[i] = 104; // 'h'

    hexDump("buffer", buffer, arena.offset);

    Data *data = make(Data, 1, allocator);
    data->x = 1;
    data->y = 2;
    data->z = 'a';

    hexDump("buffer", buffer, arena.offset);

    release(Data, 1, data, allocator);
    release(char, 100, z, allocator);
    release(char, 78, new_z, allocator);
    release(int, 400, new_x, allocator);
    release(size_t, 24, new_y, allocator);
    release(int, 92, new_zerod, allocator);

    assert(allocated(allocator) == 0, "Memory leak detected, allocated: %zu\n", allocated(allocator));

    arena_free_all(&arena);

    free(buffer);
    buffer = NULL;

    info("Arena test passed\n");

    return 0;
}
