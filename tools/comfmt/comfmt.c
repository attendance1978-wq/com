/* comfmt - canonical formatter for com source.
 * usage: comfmt [-w] file.com     (prints to stdout; -w rewrites the file) */
#include "lexer/lexer.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    StrBuf out;
    int indent;
    int at_start;
    int pending; /* a newline is owed before the next token */
} Fmt;

static void newline(Fmt *f) {
    sb_putc(&f->out, '\n');
    f->at_start = 1;
    f->pending = 0;
}

static void pad(Fmt *f) {
    for (int i = 0; i < f->indent; i++) sb_puts(&f->out, "    ");
    f->at_start = 0;
}

static void put_token(Fmt *f, const Token *t) {
    if (t->type == TOK_NUM) {
        for (long i = 0; i < t->value; i++) sb_putc(&f->out, '1');
    } else if (t->type == TOK_IDENT || t->type == TOK_COMMENT) {
        for (int i = 0; i < t->len; i++) sb_putc(&f->out, t->start[i]);
    } else {
        sb_puts(&f->out, token_text(t->type));
    }
}

static char *format(const char *src, ComError *err) {
    TokenList tl;
    if (lex(src, &tl, err, 1)) return NULL;

    Fmt f;
    sb_init(&f.out);
    f.indent = 0;
    f.at_start = 1;
    f.pending = 0;

    TokenType prev = TOK_EOF;
    int prev_unary = 0, last_line = 0;

    for (int i = 0; i < tl.count; i++) {
        const Token *t = &tl.toks[i];
        if (t->type == TOK_EOF) break;

        if (t->type == TOK_COMMENT) {
            if (f.pending && t->line == last_line) {
                sb_puts(&f.out, "  "); /* trailing comment */
            } else {
                if (f.pending || !f.at_start) newline(&f);
                pad(&f);
            }
            put_token(&f, t);
            f.pending = 1;
            last_line = t->line;
            continue;
        }

        if (f.pending) newline(&f);
        if (t->type == TOK_RBRACE && f.indent > 0) f.indent--;

        if (f.at_start) {
            pad(&f);
        } else {
            int space = 1;
            if (t->type == TOK_SEMI || t->type == TOK_RPAREN) space = 0;
            if (prev == TOK_LPAREN || prev_unary) space = 0;
            if (space) sb_putc(&f.out, ' ');
        }

        int unary = t->type == TOK_MINUS && prev != TOK_NUM && prev != TOK_IDENT && prev != TOK_RPAREN;
        put_token(&f, t);

        if (t->type == TOK_SEMI) f.pending = 1;
        if (t->type == TOK_LBRACE) { f.pending = 1; f.indent++; }
        if (t->type == TOK_RBRACE && !(i + 1 < tl.count && tl.toks[i + 1].type == TOK_ELSE)) f.pending = 1;

        prev = t->type;
        prev_unary = unary;
        last_line = t->line;
    }
    if (f.pending || !f.at_start) newline(&f);

    token_list_free(&tl);
    return sb_take(&f.out);
}

int main(int argc, char **argv) {
    int write = 0;
    const char *path = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) write = 1;
        else path = argv[i];
    }
    if (!path) {
        fputs("usage: comfmt [-w] file.com\n", stderr);
        return 2;
    }
    char *src = str_read_file(path);
    if (!src) {
        fprintf(stderr, "comfmt: cannot read '%s'\n", path);
        return 1;
    }
    ComError err;
    char *out = format(src, &err);
    if (!out) {
        error_print(&err, path);
        free(src);
        return 1;
    }
    int rc = 0;
    if (write) {
        FILE *fp = fopen(path, "wb");
        if (!fp) {
            fprintf(stderr, "comfmt: cannot write '%s'\n", path);
            rc = 1;
        } else {
            fputs(out, fp);
            fclose(fp);
        }
    } else {
        fputs(out, stdout);
    }
    free(out);
    free(src);
    return rc;
}
