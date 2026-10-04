#include "parser/parser.h"

#include <setjmp.h>
#include <stdarg.h>
#include <string.h>

typedef struct {
    const Token *toks;
    int pos;
    Arena *arena;
    ComError *err;
    jmp_buf jb;
} Parser;

static const Token *peek(Parser *p) { return &p->toks[p->pos]; }

static const Token *advance(Parser *p) {
    const Token *t = &p->toks[p->pos];
    if (t->type != TOK_EOF) p->pos++;
    return t;
}

static void fail(Parser *p, const Token *t, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    error_vset(p->err, t->line, t->col, fmt, ap);
    va_end(ap);
    longjmp(p->jb, 1);
}

static const Token *expect(Parser *p, TokenType ty, const char *ctx) {
    const Token *t = peek(p);
    if (t->type != ty)
        fail(p, t, "expected %s %s, found %s", token_name(ty), ctx, token_name(t->type));
    return advance(p);
}

static Expr *new_expr(Parser *p, ExprKind k, const Token *at) {
    Expr *e = arena_alloc(p->arena, sizeof *e);
    e->kind = k;
    e->line = at->line;
    e->col = at->col;
    return e;
}

static Stmt *new_stmt(Parser *p, StmtKind k, const Token *at) {
    Stmt *s = arena_alloc(p->arena, sizeof *s);
    s->kind = k;
    s->line = at->line;
    s->col = at->col;
    return s;
}

static void copy_name(char *dst, const Token *t) {
    memcpy(dst, t->start, (size_t)t->len);
    dst[t->len] = '\0';
}

static Expr *parse_expr(Parser *p);
static Stmt *parse_stmt(Parser *p);

static Expr *mk_bin(Parser *p, BinOp op, Expr *l, Expr *r, const Token *at) {
    Expr *e = new_expr(p, EXPR_BIN, at);
    e->op = op;
    e->lhs = l;
    e->rhs = r;
    return e;
}

static Expr *parse_primary(Parser *p) {
    const Token *t = peek(p);
    switch (t->type) {
    case TOK_NUM: {
        advance(p);
        Expr *e = new_expr(p, EXPR_NUM, t);
        e->num = t->value;
        return e;
    }
    case TOK_IDENT: {
        advance(p);
        Expr *e = new_expr(p, EXPR_VAR, t);
        copy_name(e->name, t);
        return e;
    }
    case TOK_LPAREN: {
        advance(p);
        Expr *e = parse_expr(p);
        expect(p, TOK_RPAREN, "to close '('");
        return e;
    }
    case TOK_MINUS: { /* unary minus: 0 - x */
        advance(p);
        Expr *zero = new_expr(p, EXPR_NUM, t);
        Expr *rhs = parse_primary(p);
        return mk_bin(p, BIN_SUB, zero, rhs, t);
    }
    default:
        fail(p, t, "expected an expression, found %s", token_name(t->type));
    }
    return NULL; /* unreachable */
}

static Expr *parse_mul(Parser *p) {
    Expr *l = parse_primary(p);
    for (;;) {
        const Token *t = peek(p);
        BinOp op;
        if (t->type == TOK_STAR) op = BIN_MUL;
        else if (t->type == TOK_SLASH) op = BIN_DIV;
        else if (t->type == TOK_PERCENT) op = BIN_MOD;
        else return l;
        advance(p);
        l = mk_bin(p, op, l, parse_primary(p), t);
    }
}

static Expr *parse_add(Parser *p) {
    Expr *l = parse_mul(p);
    for (;;) {
        const Token *t = peek(p);
        BinOp op;
        if (t->type == TOK_PLUS) op = BIN_ADD;
        else if (t->type == TOK_MINUS) op = BIN_SUB;
        else return l;
        advance(p);
        l = mk_bin(p, op, l, parse_mul(p), t);
    }
}

static Expr *parse_expr(Parser *p) {
    Expr *l = parse_add(p);
    for (;;) {
        const Token *t = peek(p);
        BinOp op;
        if (t->type == TOK_LT) op = BIN_LT;
        else if (t->type == TOK_GT) op = BIN_GT;
        else if (t->type == TOK_EQEQ) op = BIN_EQ;
        else if (t->type == TOK_NEQ) op = BIN_NE;
        else return l;
        advance(p);
        l = mk_bin(p, op, l, parse_add(p), t);
    }
}

static Stmt *parse_block(Parser *p) {
    const Token *lb = expect(p, TOK_LBRACE, "to start a block");
    Stmt *blk = new_stmt(p, STMT_BLOCK, lb);
    Stmt **tail = &blk->body;
    while (peek(p)->type != TOK_RBRACE) {
        if (peek(p)->type == TOK_EOF) fail(p, peek(p), "unterminated block: expected '}'");
        Stmt *s = parse_stmt(p);
        *tail = s;
        tail = &s->next;
    }
    advance(p);
    return blk;
}

static Stmt *parse_stmt(Parser *p) {
    const Token *t = peek(p);
    switch (t->type) {
    case TOK_LET: {
        advance(p);
        const Token *id = expect(p, TOK_IDENT, "after 'let'");
        expect(p, TOK_ASSIGN, "after the variable name");
        Stmt *s = new_stmt(p, STMT_LET, t);
        copy_name(s->name, id);
        s->expr = parse_expr(p);
        expect(p, TOK_SEMI, "after 'let' statement");
        return s;
    }
    case TOK_IDENT: {
        advance(p);
        Stmt *s = new_stmt(p, STMT_ASSIGN, t);
        copy_name(s->name, t);
        expect(p, TOK_ASSIGN, "after the variable name");
        s->expr = parse_expr(p);
        expect(p, TOK_SEMI, "after assignment");
        return s;
    }
    case TOK_PRINT:
    case TOK_PRINTC: {
        advance(p);
        Stmt *s = new_stmt(p, t->type == TOK_PRINT ? STMT_PRINT : STMT_PRINTC, t);
        s->expr = parse_expr(p);
        expect(p, TOK_SEMI, "after print statement");
        return s;
    }
    case TOK_WHILE: {
        advance(p);
        Stmt *s = new_stmt(p, STMT_WHILE, t);
        s->expr = parse_expr(p);
        s->body = parse_block(p);
        return s;
    }
    case TOK_IF: {
        advance(p);
        Stmt *s = new_stmt(p, STMT_IF, t);
        s->expr = parse_expr(p);
        s->body = parse_block(p);
        if (peek(p)->type == TOK_ELSE) {
            advance(p);
            if (peek(p)->type == TOK_IF) { /* else if: wrap in a block */
                Stmt *blk = new_stmt(p, STMT_BLOCK, peek(p));
                blk->body = parse_stmt(p);
                s->else_body = blk;
            } else {
                s->else_body = parse_block(p);
            }
        }
        return s;
    }
    case TOK_LBRACE:
        return parse_block(p);
    default:
        fail(p, t, "expected a statement, found %s", token_name(t->type));
    }
    return NULL; /* unreachable */
}

int parse(const TokenList *toks, Arena *arena, Program *out, ComError *err) {
    Parser p;
    p.toks = toks->toks;
    p.pos = 0;
    p.arena = arena;
    p.err = err;
    out->first = NULL;
    if (setjmp(p.jb)) return 1;

    Stmt **tail = &out->first;
    while (peek(&p)->type != TOK_EOF) {
        Stmt *s = parse_stmt(&p);
        *tail = s;
        tail = &s->next;
    }
    return 0;
}
