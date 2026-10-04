#include "cli/cli.h"

#include "codegen/codegen.h"
#include "com/version.h"
#include "runtime/runtime.h"
#include "util/error.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WITH 16

typedef struct {
    const char *file;
    const char *out;
    const char *emit;
    const char *with[MAX_WITH];
    int nwith;
    int opt;
} Options;

static void usage(FILE *f) {
    fputs("com " COM_VERSION_STRING " - a base-1 (unary) programming language\n"
          "\n"
          "usage:\n"
          "  com run [options] <file.com|file.one>   run a program\n"
          "  com build [options] <file.com>          compile a program\n"
          "  com repl                                interactive prompt\n"
          "  com version | help\n"
          "\n"
          "options:\n"
          "  --with <file.com>   prepend a file (e.g. stdlib/core.com); repeatable.\n"
          "                      line numbers in errors refer to the combined text\n"
          "  --no-opt            disable constant folding\n"
          "  --emit c|one|ir     (build) output kind, default c\n"
          "  -o <file>           (build) write output to a file instead of stdout\n",
          f);
}

static int parse_options(int argc, char **argv, Options *o) {
    memset(o, 0, sizeof *o);
    o->opt = 1;
    o->emit = "c";
    for (int i = 0; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "--no-opt") == 0) {
            o->opt = 0;
        } else if (strcmp(a, "--with") == 0 || strcmp(a, "--emit") == 0 || strcmp(a, "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "com: option %s needs a value\n", a);
                return 1;
            }
            const char *v = argv[++i];
            if (strcmp(a, "--with") == 0) {
                if (o->nwith >= MAX_WITH) {
                    fprintf(stderr, "com: too many --with files (max %d)\n", MAX_WITH);
                    return 1;
                }
                o->with[o->nwith++] = v;
            } else if (strcmp(a, "--emit") == 0) {
                o->emit = v;
            } else {
                o->out = v;
            }
        } else if (a[0] == '-' && a[1] != '\0') {
            fprintf(stderr, "com: unknown option '%s'\n", a);
            return 1;
        } else if (!o->file) {
            o->file = a;
        } else {
            fprintf(stderr, "com: unexpected argument '%s'\n", a);
            return 1;
        }
    }
    if (!o->file) {
        fputs("com: no input file\n", stderr);
        return 1;
    }
    return 0;
}

static char *read_or_die(const char *path) {
    char *s = str_read_file(path);
    if (!s) fprintf(stderr, "com: cannot read '%s'\n", path);
    return s;
}

/* --with files first, then the main file */
static char *load_program(const Options *o) {
    StrBuf sb;
    sb_init(&sb);
    for (int i = 0; i < o->nwith; i++) {
        char *w = read_or_die(o->with[i]);
        if (!w) { sb_free(&sb); return NULL; }
        sb_puts(&sb, w);
        sb_putc(&sb, '\n');
        free(w);
    }
    char *m = read_or_die(o->file);
    if (!m) { sb_free(&sb); return NULL; }
    sb_puts(&sb, m);
    free(m);
    return sb_take(&sb);
}

static int cmd_run(int argc, char **argv) {
    Options o;
    if (parse_options(argc, argv, &o)) return 1;

    if (str_ends_with(o.file, ".one")) {
        char *text = read_or_die(o.file);
        if (!text) return 1;
        IR ir;
        ComError err;
        long vars[COM_MAX_VARS] = {0};
        int rc = one_load(text, &ir, &err);
        if (rc) {
            error_print(&err, o.file);
        } else {
            rc = vm_run(&ir, vars, COM_MAX_VARS, &err);
            if (rc) error_print(&err, o.file);
        }
        ir_free(&ir);
        free(text);
        return rc;
    }

    char *src = load_program(&o);
    if (!src) return 1;
    int rc = com_run_source_opt(src, o.file, o.opt);
    free(src);
    return rc;
}

static int cmd_build(int argc, char **argv) {
    Options o;
    EmitKind kind;
    if (parse_options(argc, argv, &o)) return 1;

    if (strcmp(o.emit, "c") == 0) kind = EMIT_C;
    else if (strcmp(o.emit, "one") == 0) kind = EMIT_ONE;
    else if (strcmp(o.emit, "ir") == 0) kind = EMIT_IR;
    else {
        fprintf(stderr, "com: unknown --emit kind '%s' (use c, one or ir)\n", o.emit);
        return 1;
    }

    char *src = load_program(&o);
    if (!src) return 1;
    char *out = com_emit(src, o.file, kind, o.opt);
    free(src);
    if (!out) return 1;

    int rc = 0;
    if (o.out) {
        FILE *f = fopen(o.out, "wb");
        if (!f) {
            fprintf(stderr, "com: cannot write '%s'\n", o.out);
            rc = 1;
        } else {
            fputs(out, f);
            fclose(f);
        }
    } else {
        fputs(out, stdout);
    }
    free(out);
    return rc;
}

int cli_main(int argc, char **argv) {
    if (argc < 2) {
        usage(stderr);
        return 1;
    }
    const char *cmd = argv[1];
    if (strcmp(cmd, "run") == 0) return cmd_run(argc - 2, argv + 2);
    if (strcmp(cmd, "build") == 0) return cmd_build(argc - 2, argv + 2);
    if (strcmp(cmd, "repl") == 0) return repl_run();
    if (strcmp(cmd, "version") == 0 || strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
        puts("com " COM_VERSION_STRING);
        return 0;
    }
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
        usage(stdout);
        return 0;
    }
    /* `com file.com` is shorthand for `com run file.com` */
    return cmd_run(argc - 1, argv + 1);
}
