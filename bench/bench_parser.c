/* Parser throughput: tokenise once, parse repeatedly. */
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    StrBuf sb;
    sb_init(&sb);
    for (int i = 0; i < 20000; i++)
        sb_puts(&sb, "let v = 1111111111 * 111 + (11 - 1); while v > 1 { v = v - 1; }\n");
    char *src = sb_take(&sb);

    TokenList tl;
    ComError err;
    if (lex(src, &tl, &err, 0)) { fprintf(stderr, "lex failed\n"); return 1; }

    const int iters = 20;
    clock_t t0 = clock();
    for (int i = 0; i < iters; i++) {
        Arena a;
        Program p;
        arena_init(&a);
        if (parse(&tl, &a, &p, &err)) { fprintf(stderr, "parse failed\n"); return 1; }
        arena_free(&a);
    }
    double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("bench_parser: %d tokens, %.1f Mtokens/s\n", tl.count, (double)tl.count * iters / secs / 1e6);
    token_list_free(&tl);
    free(src);
    return 0;
}
