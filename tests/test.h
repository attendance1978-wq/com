#ifndef COM_TEST_H
#define COM_TEST_H

#include <stdio.h>

static int t_run, t_fail;

#define CHECK(cond)                                                            \
    do {                                                                       \
        t_run++;                                                               \
        if (!(cond)) {                                                         \
            t_fail++;                                                          \
            fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                      \
    } while (0)

#define TEST_DONE()                                                            \
    do {                                                                       \
        printf("%-34s %3d checks, %d failed\n", __FILE__, t_run, t_fail);      \
        return t_fail ? 1 : 0;                                                 \
    } while (0)

#endif
