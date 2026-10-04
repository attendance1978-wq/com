/* comlint - report errors and unused variables.
 * usage: comlint file.com [more.com ...]   (exit 1 if anything was reported) */
#include "lexer/lexer.h"
#include "runtime/runtime.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int same_ident(const Token *a, const Token *b) {
    return a->len == b->len && memcmp(a->start, b->start, (size_t)a->len) == 0;
}

static int lint_file(const char *path) {
    char *src = str_read_file(path);
    if (!src) {
        fprintf(stderr, "comlint: cannot read '%s'\n", path);
        return 1;
    }
    int problems = 0;

    /* errors: run the normal front end */
    SymbolTable st;
    IR ir;
    ComError err;
    symtab_init(&st);
    if (com_compile(src, &ir, &st, &err, 0)) {
        error_print(&err, path);
        problems++;
    }
    ir_free(&ir);

    /* warnings: variables declared but never mentioned again */
    TokenList tl;
    if (problems == 0 && lex(src, &tl, &err, 0) == 0) {
        for (int i = 0; i + 1 < tl.count; i++) {
            if (tl.toks[i].type != TOK_LET || tl.toks[i + 1].type != TOK_IDENT) continue;
            const Token *decl = &tl.toks[i + 1];
            int uses = 0;
            for (int j = 0; j < tl.count; j++)
                if (j != i + 1 && tl.toks[j].type == TOK_IDENT && same_ident(decl, &tl.toks[j])) uses++;
            if (uses == 0) {
                printf("%s:%d:%d: warning: variable '%.*s' is never used\n", path, decl->line,
                       decl->col, decl->len, decl->start);
                problems++;
            }
        }
        token_list_free(&tl);
    }
    free(src);
    return problems;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs("usage: comlint file.com [more.com ...]\n", stderr);
        return 2;
    }
    int total = 0;
    for (int i = 1; i < argc; i++) total += lint_file(argv[i]);
    if (total == 0) puts("comlint: no problems found");
    return total ? 1 : 0;
}
