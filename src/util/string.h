#ifndef COM_UTIL_STRING_H
#define COM_UTIL_STRING_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

void sb_init(StrBuf *sb);
void sb_putc(StrBuf *sb, char c);
void sb_puts(StrBuf *sb, const char *s);
void sb_printf(StrBuf *sb, const char *fmt, ...);
/* Returns the malloc'd, NUL-terminated buffer and resets the StrBuf. */
char *sb_take(StrBuf *sb);
void sb_free(StrBuf *sb);

char *str_dup(const char *s);
/* Reads a whole file into a malloc'd string, or NULL on failure. */
char *str_read_file(const char *path);
int str_ends_with(const char *s, const char *suffix);

#endif
