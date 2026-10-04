#include "test.h"
#include "lexer/lexer.h"

#include <string.h>

int main(void) {
    TokenList tl;
    ComError err;

    CHECK(lex("let x = 111;", &tl, &err, 0) == 0);
    CHECK(tl.count == 6);
    CHECK(tl.toks[0].type == TOK_LET);
    CHECK(tl.toks[1].type == TOK_IDENT && tl.toks[1].len == 1);
    CHECK(tl.toks[2].type == TOK_ASSIGN);
    CHECK(tl.toks[3].type == TOK_NUM && tl.toks[3].value == 3);
    CHECK(tl.toks[4].type == TOK_SEMI);
    CHECK(tl.toks[5].type == TOK_EOF);
    token_list_free(&tl);

    CHECK(lex("a == b != c", &tl, &err, 0) == 0);
    CHECK(tl.toks[1].type == TOK_EQEQ && tl.toks[3].type == TOK_NEQ);
    token_list_free(&tl);

    /* only the digit 1 exists */
    CHECK(lex("12", &tl, &err, 0) == 1);
    CHECK(strstr(err.msg, "base 1") != NULL);
    CHECK(lex("0", &tl, &err, 0) == 1);
    CHECK(lex("1a", &tl, &err, 0) == 1);
    CHECK(lex("x ! y", &tl, &err, 0) == 1);
    CHECK(lex("a @ b", &tl, &err, 0) == 1);
    CHECK(err.line == 1 && err.col == 3);

    /* names may contain digits after the first character */
    CHECK(lex("x1 _a2", &tl, &err, 0) == 0);
    CHECK(tl.toks[0].type == TOK_IDENT && tl.toks[0].len == 2);
    token_list_free(&tl);

    /* comments */
    CHECK(lex("// hi\n1", &tl, &err, 0) == 0);
    CHECK(tl.count == 2 && tl.toks[0].type == TOK_NUM && tl.toks[0].line == 2);
    token_list_free(&tl);
    CHECK(lex("// hi\n1", &tl, &err, 1) == 0);
    CHECK(tl.count == 3 && tl.toks[0].type == TOK_COMMENT && tl.toks[0].len == 5);
    token_list_free(&tl);

    TEST_DONE();
}
