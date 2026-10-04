#include "sema/analyzer.h"

static int an_stmt(Stmt *s, SymbolTable *st, ComError *err);

static int an_expr(Expr *e, SymbolTable *st, ComError *err) {
    switch (e->kind) {
    case EXPR_NUM:
        return 0;
    case EXPR_VAR:
        e->slot = symtab_lookup(st, e->name);
        if (e->slot < 0) {
            error_set(err, e->line, e->col, "undefined variable '%s'", e->name);
            return 1;
        }
        return 0;
    case EXPR_BIN:
        return an_expr(e->lhs, st, err) || an_expr(e->rhs, st, err);
    }
    return 0;
}

static int an_stmts(Stmt *s, SymbolTable *st, ComError *err) {
    for (; s; s = s->next)
        if (an_stmt(s, st, err)) return 1;
    return 0;
}

static int an_stmt(Stmt *s, SymbolTable *st, ComError *err) {
    switch (s->kind) {
    case STMT_LET:
        /* analyse the value first so `let x = x;` is an error */
        if (an_expr(s->expr, st, err)) return 1;
        if (symtab_lookup(st, s->name) >= 0) {
            error_set(err, s->line, s->col, "variable '%s' is already declared", s->name);
            return 1;
        }
        s->slot = symtab_declare(st, s->name);
        if (s->slot < 0) {
            error_set(err, s->line, s->col, "too many variables (max %d)", COM_MAX_VARS);
            return 1;
        }
        return 0;
    case STMT_ASSIGN:
        s->slot = symtab_lookup(st, s->name);
        if (s->slot < 0) {
            error_set(err, s->line, s->col, "assignment to undefined variable '%s' (declare it with 'let')", s->name);
            return 1;
        }
        return an_expr(s->expr, st, err);
    case STMT_PRINT:
    case STMT_PRINTC:
        return an_expr(s->expr, st, err);
    case STMT_WHILE:
        return an_expr(s->expr, st, err) || an_stmt(s->body, st, err);
    case STMT_IF:
        if (an_expr(s->expr, st, err) || an_stmt(s->body, st, err)) return 1;
        return s->else_body ? an_stmt(s->else_body, st, err) : 0;
    case STMT_BLOCK:
        return an_stmts(s->body, st, err);
    }
    return 0;
}

int analyze(Program *prog, SymbolTable *st, ComError *err) {
    return an_stmts(prog->first, st, err);
}
