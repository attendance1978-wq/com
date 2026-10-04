#include "cli/cli.h"

#include "com/version.h"
#include "lexer/lexer.h"
#include "runtime/runtime.h"
#include "util/error.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Brace depth of `src`; -1 if it does not lex (let the compiler report it). */
static int brace_depth(const char *src) {
    TokenList tl;
    ComError err;
    if (lex(src, &tl, &err, 0)) return -1;
    int depth = 0;
    for (int i = 0; i < tl.count; i++) {
        if (tl.toks[i].type == TOK_LBRACE) depth++;
        if (tl.toks[i].type == TOK_RBRACE) depth--;
    }
    token_list_free(&tl);
    return depth;
}

/* Turns a bare expression into `print <expr>;` and adds a missing ';'. */
static char *prepare(const char *line) {
    TokenList tl;
    ComError err;
    StrBuf sb;
    sb_init(&sb);

    if (lex(line, &tl, &err, 0) || tl.count < 2) {
        if (tl.toks) token_list_free(&tl);
        sb_puts(&sb, line);
        return sb_take(&sb);
    }
    TokenType first = tl.toks[0].type;
    TokenType second = tl.toks[1].type;
    TokenType last = tl.toks[tl.count - 2].type;
    int bare = first == TOK_NUM || first == TOK_LPAREN || first == TOK_MINUS ||
               (first == TOK_IDENT && second != TOK_ASSIGN);

    if (bare) sb_puts(&sb, "print ");
    sb_puts(&sb, line);
    if (last != TOK_SEMI && last != TOK_RBRACE) sb_putc(&sb, ';');
    token_list_free(&tl);
    return sb_take(&sb);
}

static void print_vars(const SymbolTable *st, const long *vars) {
    if (st->count == 0) puts("(no variables)");
    for (int i = 0; i < st->count; i++) printf("%s = %ld\n", st->names[i], vars[i]);
}

int repl_run(void) {
    SymbolTable st;
    long vars[COM_MAX_VARS] = {0};
    char line[1024];

    symtab_init(&st);
    puts("com " COM_VERSION_STRING " repl - numbers are written in 1s. :help for help, :quit to exit.");

    for (;;) {
        StrBuf buf;
        sb_init(&buf);
        const char *prompt = "com> ";

        for (;;) {
            fputs(prompt, stdout);
            fflush(stdout);
            if (!fgets(line, sizeof line, stdin)) {
                sb_free(&buf);
                putchar('\n');
                return 0;
            }
            sb_puts(&buf, line);
            if (brace_depth(buf.data) <= 0) break;
            prompt = "...> ";
        }

        /* trim */
        char *text = buf.data;
        while (*text == ' ' || *text == '\t' || *text == '\n' || *text == '\r') text++;
        size_t n = strlen(text);
        while (n > 0 && (text[n - 1] == '\n' || text[n - 1] == '\r' || text[n - 1] == ' ')) text[--n] = '\0';

        if (n == 0) { sb_free(&buf); continue; }
        if (strcmp(text, ":quit") == 0 || strcmp(text, ":q") == 0 || strcmp(text, ":exit") == 0) {
            sb_free(&buf);
            return 0;
        }
        if (strcmp(text, ":vars") == 0) {
            print_vars(&st, vars);
        } else if (strcmp(text, ":help") == 0) {
            puts("  <statement>   run it (let x = 111; print x; while ... { }  if ... { })\n"
                 "  <expression>  evaluate and print it, e.g. 11 * 111\n"
                 "  :vars         list variables\n"
                 "  :quit         exit");
        } else {
            char *src = prepare(text);
            SymbolTable backup = st;
            IR ir;
            ComError err;
            if (com_compile(src, &ir, &st, &err, 1)) {
                error_print(&err, "<repl>");
                st = backup; /* forget declarations from a failed line */
            } else if (vm_run(&ir, vars, COM_MAX_VARS, &err)) {
                error_print(&err, "<repl>");
            }
            ir_free(&ir);
            free(src);
        }
        sb_free(&buf);
    }
}
