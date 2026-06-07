#ifndef CTEST_H
#define CTEST_H

#include <stdio.h>

#define CTEST_ASSERT(expr)                                                     \
    do {                                                                       \
        if (!(expr)) {                                                         \
            printf("Assertion failed: %s, file %s, line %d\n",                 \
                   #expr, __FILE__, __LINE__);                                 \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define CTEST_RUN(test_name)                                                   \
    do {                                                                       \
        int result = test_name();                                              \
        if (result == 0) {                                                     \
            printf("[PASS] %s\n", #test_name);                                \
        } else {                                                               \
            printf("[FAIL] %s\n", #test_name);                                \
            failures++;                                                        \
        }                                                                      \
    } while (0)

#endif