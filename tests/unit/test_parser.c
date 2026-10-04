#include "test.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

static int parse_src(const char *src, Arena *a, Program *p, ComError *err) {
    TokenList tl;
    if (lex(src, &tl, err, 0)) return 1;
    arena_init(a);
    int rc = parse(&tl, a, p, err);
    token_list_free(&tl);
    return rc;
}

int main(void) {
    Arena a;
    Program p;
    ComError err;

    CHECK(parse_src("let x = 1 + 11 * 111; print x;", &a, &p, &err) == 0);
    Stmt *s = p.first;
    CHECK(s && s->kind == STMT_LET);
    CHECK(s->expr->kind == EXPR_BIN && s->expr->op == BIN_ADD);       /* + is the root */
    CHECK(s->expr->rhs->kind == EXPR_BIN && s->expr->rhs->op == BIN_MUL); /* * binds tighter */
    CHECK(s->next && s->next->kind == STMT_PRINT && s->next->next == NULL);
    arena_free(&a);

    CHECK(parse_src("while 1 < 11 { x = 1; } if 1 { } else if 1 { } else { }", &a, &p, &err) == 0);
    CHECK(p.first->kind == STMT_WHILE && p.first->body->kind == STMT_BLOCK);
    CHECK(p.first->next->kind == STMT_IF && p.first->next->else_body != NULL);
    arena_free(&a);

    CHECK(parse_src("print -11;", &a, &p, &err) == 0);
    CHECK(p.first->expr->kind == EXPR_BIN && p.first->expr->op == BIN_SUB);
    arena_free(&a);

    CHECK(parse_src("let = 1;", &a, &p, &err) == 1);
    arena_free(&a);
    CHECK(parse_src("let x = 1", &a, &p, &err) == 1);
    CHECK(err.line == 1);
    arena_free(&a);
    CHECK(parse_src("while 1 { print 1;", &a, &p, &err) == 1);
    arena_free(&a);
    CHECK(parse_src("print (1 + 1;", &a, &p, &err) == 1);
    arena_free(&a);

    TEST_DONE();
}
