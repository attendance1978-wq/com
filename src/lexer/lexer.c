#include "lexer/lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static const char *const raw_text[TOK_COUNT_] = {
    [TOK_LET] = "let",     [TOK_PRINT] = "print",   [TOK_PRINTC] = "printc",
    [TOK_WHILE] = "while", [TOK_IF] = "if",         [TOK_ELSE] = "else",
    [TOK_PLUS] = "+",      [TOK_MINUS] = "-",       [TOK_STAR] = "*",
    [TOK_SLASH] = "/",     [TOK_PERCENT] = "%",     [TOK_LT] = "<",
    [TOK_GT] = ">",        [TOK_EQEQ] = "==",       [TOK_NEQ] = "!=",
    [TOK_ASSIGN] = "=",    [TOK_LPAREN] = "(",      [TOK_RPAREN] = ")",
    [TOK_LBRACE] = "{",    [TOK_RBRACE] = "}",      [TOK_SEMI] = ";",
};

static const char *const display[TOK_COUNT_] = {
    [TOK_EOF] = "end of file", [TOK_NUM] = "number",   [TOK_IDENT] = "identifier",
    [TOK_COMMENT] = "comment", [TOK_LET] = "'let'",    [TOK_PRINT] = "'print'",
    [TOK_PRINTC] = "'printc'", [TOK_WHILE] = "'while'", [TOK_IF] = "'if'",
    [TOK_ELSE] = "'else'",     [TOK_PLUS] = "'+'",     [TOK_MINUS] = "'-'",
    [TOK_STAR] = "'*'",        [TOK_SLASH] = "'/'",    [TOK_PERCENT] = "'%'",
    [TOK_LT] = "'<'",          [TOK_GT] = "'>'",       [TOK_EQEQ] = "'=='",
    [TOK_NEQ] = "'!='",        [TOK_ASSIGN] = "'='",   [TOK_LPAREN] = "'('",
    [TOK_RPAREN] = "')'",      [TOK_LBRACE] = "'{'",   [TOK_RBRACE] = "'}'",
    [TOK_SEMI] = "';'",
};

const char *token_text(TokenType t) { return raw_text[t]; }
const char *token_name(TokenType t) { return display[t]; }

void token_list_free(TokenList *l) {
    free(l->toks);
    l->toks = NULL;
    l->count = 0;
    l->cap = 0;
}

static int add(TokenList *l, Token t) {
    if (l->count == l->cap) {
        int nc = l->cap ? l->cap * 2 : 64;
        Token *n = realloc(l->toks, (size_t)nc * sizeof(Token));
        if (!n) return 1;
        l->toks = n;
        l->cap = nc;
    }
    l->toks[l->count++] = t;
    return 0;
}

static TokenType keyword(const char *s, int len) {
    static const TokenType kws[] = {TOK_LET, TOK_PRINT, TOK_PRINTC, TOK_WHILE, TOK_IF, TOK_ELSE};
    for (size_t i = 0; i < sizeof kws / sizeof kws[0]; i++) {
        const char *k = raw_text[kws[i]];
        if ((int)strlen(k) == len && memcmp(k, s, (size_t)len) == 0) return kws[i];
    }
    return TOK_IDENT;
}

#define ADV()                                                                  \
    do {                                                                       \
        if (*p == '\n') { line++; col = 1; } else { col++; }                   \
        p++;                                                                   \
    } while (0)

#define FAIL(l, c, ...)                                                        \
    do {                                                                       \
        error_set(err, (l), (c), __VA_ARGS__);                                 \
        token_list_free(out);                                                  \
        return 1;                                                              \
    } while (0)

#define ADD(tok)                                                               \
    do {                                                                       \
        if (add(out, (tok))) { token_list_free(out); error_set(err, 0, 0, "out of memory"); return 1; } \
    } while (0)

int lex(const char *src, TokenList *out, ComError *err, int keep_comments) {
    out->toks = NULL;
    out->count = 0;
    out->cap = 0;

    const char *p = src;
    int line = 1, col = 1;

    while (*p) {
        char c = *p;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            ADV();
            continue;
        }

        Token t;
        memset(&t, 0, sizeof t);
        t.line = line;
        t.col = col;
        t.start = p;

        if (c == '/' && p[1] == '/') {
            const char *s = p;
            while (*p && *p != '\n') ADV();
            if (keep_comments) {
                int len = (int)(p - s);
                while (len > 0 && s[len - 1] == '\r') len--;
                t.type = TOK_COMMENT;
                t.len = len;
                ADD(t);
            }
            continue;
        }

        if (c == '1') {
            long n = 0;
            while (*p == '1') {
                n++;
                ADV();
            }
            if (isdigit((unsigned char)*p))
                FAIL(line, col, "invalid digit '%c': base 1 has only the digit '1'", *p);
            if (isalpha((unsigned char)*p) || *p == '_')
                FAIL(line, col, "a number cannot be followed by '%c' (names cannot start with a digit)", *p);
            t.type = TOK_NUM;
            t.value = n;
            t.len = (int)n;
            ADD(t);
            continue;
        }

        if (isdigit((unsigned char)c))
            FAIL(line, col, "invalid digit '%c': base 1 has only the digit '1'", c);

        if (isalpha((unsigned char)c) || c == '_') {
            const char *s = p;
            while (isalnum((unsigned char)*p) || *p == '_') ADV();
            int len = (int)(p - s);
            if (len > 63) FAIL(t.line, t.col, "identifier too long (max 63 characters)");
            t.type = keyword(s, len);
            t.len = len;
            ADD(t);
            continue;
        }

        TokenType ty = TOK_EOF;
        int len = 1;
        switch (c) {
        case '+': ty = TOK_PLUS; break;
        case '-': ty = TOK_MINUS; break;
        case '*': ty = TOK_STAR; break;
        case '/': ty = TOK_SLASH; break;
        case '%': ty = TOK_PERCENT; break;
        case '<': ty = TOK_LT; break;
        case '>': ty = TOK_GT; break;
        case '(': ty = TOK_LPAREN; break;
        case ')': ty = TOK_RPAREN; break;
        case '{': ty = TOK_LBRACE; break;
        case '}': ty = TOK_RBRACE; break;
        case ';': ty = TOK_SEMI; break;
        case '=':
            if (p[1] == '=') { ty = TOK_EQEQ; len = 2; } else ty = TOK_ASSIGN;
            break;
        case '!':
            if (p[1] == '=') { ty = TOK_NEQ; len = 2; }
            else FAIL(line, col, "unexpected '!' (did you mean '!='?)");
            break;
        default:
            if (isprint((unsigned char)c)) FAIL(line, col, "unexpected character '%c'", c);
            FAIL(line, col, "unexpected byte 0x%02x", (unsigned)(unsigned char)c);
        }
        t.type = ty;
        t.len = len;
        for (int i = 0; i < len; i++) ADV();
        ADD(t);
    }

    Token eof;
    memset(&eof, 0, sizeof eof);
    eof.type = TOK_EOF;
    eof.line = line;
    eof.col = col;
    eof.start = p;
    ADD(eof);
    return 0;
}
