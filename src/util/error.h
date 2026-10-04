#ifndef COM_UTIL_ERROR_H
#define COM_UTIL_ERROR_H

#include <stdarg.h>

typedef struct {
    int line; /* 1-based, 0 = no position */
    int col;
    char msg[256];
} ComError;

void error_vset(ComError *e, int line, int col, const char *fmt, va_list ap);
void error_set(ComError *e, int line, int col, const char *fmt, ...);
void error_print(const ComError *e, const char *filename);
void error_fatal(const char *fmt, ...);

#endif
