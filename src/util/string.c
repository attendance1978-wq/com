#include "util/string.h"
#include "util/error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sb_init(StrBuf *sb) {
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
}

static void sb_grow(StrBuf *sb, size_t extra) {
    size_t need = sb->len + extra + 1;
    if (need <= sb->cap) return;
    size_t cap = sb->cap ? sb->cap : 64;
    while (cap < need) cap *= 2;
    char *d = realloc(sb->data, cap);
    if (!d) error_fatal("out of memory");
    sb->data = d;
    sb->cap = cap;
}

void sb_putc(StrBuf *sb, char c) {
    sb_grow(sb, 1);
    sb->data[sb->len++] = c;
    sb->data[sb->len] = '\0';
}

void sb_puts(StrBuf *sb, const char *s) {
    size_t n = strlen(s);
    sb_grow(sb, n);
    memcpy(sb->data + sb->len, s, n + 1);
    sb->len += n;
}

void sb_printf(StrBuf *sb, const char *fmt, ...) {
    va_list ap, ap2;
    va_start(ap, fmt);
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) {
        va_end(ap2);
        return;
    }
    sb_grow(sb, (size_t)n);
    vsnprintf(sb->data + sb->len, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    sb->len += (size_t)n;
}

char *sb_take(StrBuf *sb) {
    if (!sb->data) {
        sb_grow(sb, 0);
        sb->data[0] = '\0';
    }
    char *d = sb->data;
    sb_init(sb);
    return d;
}

void sb_free(StrBuf *sb) {
    free(sb->data);
    sb_init(sb);
}

char *str_dup(const char *s) {
    size_t n = strlen(s) + 1;
    char *d = malloc(n);
    if (!d) error_fatal("out of memory");
    memcpy(d, s, n);
    return d;
}

char *str_read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if (!buf) error_fatal("out of memory");
    for (;;) {
        size_t got = fread(buf + len, 1, cap - len - 1, f);
        len += got;
        if (got == 0) break; /* EOF or error */
        if (len + 1 >= cap) {
            cap *= 2;
            char *n = realloc(buf, cap);
            if (!n) error_fatal("out of memory");
            buf = n;
        }
    }
    int failed = ferror(f);
    fclose(f);
    if (failed) {
        free(buf);
        return NULL;
    }
    buf[len] = '\0';
    return buf;
}

int str_ends_with(const char *s, const char *suffix) {
    size_t a = strlen(s), b = strlen(suffix);
    return a >= b && memcmp(s + a - b, suffix, b) == 0;
}
