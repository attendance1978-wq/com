#ifndef COM_API_H
#define COM_API_H

/* Public embedding API. All functions return 0 on success (or a malloc'd
 * string / NULL for the compile functions). Diagnostics go to stderr. */

/* Compile and run a program. `filename` is only used in messages. */
int com_run_source(const char *src, const char *filename);

/* Compile to a standalone C program. Caller frees the result. */
char *com_compile_to_c(const char *src, const char *filename);

/* Compile to the base-1 (unary) IR text. Caller frees the result. */
char *com_compile_to_one(const char *src, const char *filename);

#endif
