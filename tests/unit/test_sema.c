#include "test.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "sema/analyzer.h"

#include <string.h>

static int check_src(const char *src, SymbolTable *st, ComError *err) {
    TokenList tl;
    Arena a;
    Program p;
    int rc;
    if (lex(src, &tl, err, 0)) return 1;
    arena_init(&a);
    rc = parse(&tl, &a, &p, err) || analyze(&p, st, err);
    arena_free(&a);
    token_list_free(&tl);
    return rc;
}

int main(void) {
    SymbolTable st;
    ComError err;

    symtab_init(&st);
    CHECK(check_src("let a = 1; let b = a + 11; b = a;", &st, &err) == 0);
    CHECK(st.count == 2);
    CHECK(symtab_lookup(&st, "a") == 0 && symtab_lookup(&st, "b") == 1);
    CHECK(symtab_lookup(&st, "zzz") == -1);

    symtab_init(&st);
    CHECK(check_src("print y;", &st, &err) == 1);
    CHECK(strstr(err.msg, "undefined") != NULL);

    symtab_init(&st);
    CHECK(check_src("let x = 1; let x = 11;", &st, &err) == 1);
    CHECK(strstr(err.msg, "already declared") != NULL);

    symtab_init(&st);
    CHECK(check_src("let x = x;", &st, &err) == 1);

    symtab_init(&st);
    CHECK(check_src("z = 1;", &st, &err) == 1);

    /* a variable declared in one run stays visible to the next (REPL) */
    symtab_init(&st);
    CHECK(check_src("let r = 1;", &st, &err) == 0);
    CHECK(check_src("print r;", &st, &err) == 0);

    /* declarations inside blocks share the single scope */
    symtab_init(&st);
    CHECK(check_src("while 1 { let k = 1; } print k;", &st, &err) == 0);

    TEST_DONE();
}
