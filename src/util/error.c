#include "util/error.h"

#include <stdio.h>
#include <stdlib.h>

void error_vset(ComError *e, int line, int col, const char *fmt, va_list ap) {
    e->line = line;
    e->col = col;
    vsnprintf(e->msg, sizeof e->msg, fmt, ap);
}

void error_set(ComError *e, int line, int col, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    error_vset(e, line, col, fmt, ap);
    va_end(ap);
}

void error_print(const ComError *e, const char *filename) {
    if (e->line > 0)
        fprintf(stderr, "%s:%d:%d: error: %s\n", filename, e->line, e->col, e->msg);
    else
        fprintf(stderr, "%s: error: %s\n", filename, e->msg);
}

void error_fatal(const char *fmt, ...) {
    va_list ap;
    fputs("com: fatal: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}
