#ifndef _UTILS_H
#define _UTILS_H
#define is_power_of_two(x) ((x != 0) && ((x & (x - 1)) == 0))
#include <stdio.h>
#define info(...)                                                                                                      \
    fprintf(stderr, "info: %s:%d: ", __FILE__, __LINE__);                                                              \
    fprintf(stderr, __VA_ARGS__);
#define error(...)                                                                                                     \
    fprintf(stderr, "error: %s:%d: ", __FILE__, __LINE__);                                                             \
    fprintf(stderr, __VA_ARGS__);                                                                                      \
    exit(1);
#define assert(cond, ...)                                                                                              \
    if (!(cond)) {                                                                                                     \
        error(__VA_ARGS__);                                                                                            \
    }
#endif // _UTILS_H
