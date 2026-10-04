/* Lexer throughput: build a large synthetic program and tokenise it repeatedly. */
#include "lexer/lexer.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    StrBuf sb;
    sb_init(&sb);
    for (int i = 0; i < 20000; i++)
        sb_puts(&sb, "let v = 1111111111 * 111 + (11 - 1); while v > 1 { v = v - 1; } // c\n");
    char *src = sb_take(&sb);
    size_t bytes = 0;
    for (const char *p = src; *p; p++) bytes++;

    const int iters = 20;
    long tokens = 0;
    clock_t t0 = clock();
    for (int i = 0; i < iters; i++) {
        TokenList tl;
        ComError err;
        if (lex(src, &tl, &err, 0)) { fprintf(stderr, "lex failed\n"); return 1; }
        tokens = tl.count;
        token_list_free(&tl);
    }
    double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("bench_lexer : %ld tokens, %.1f MB/s, %.1f Mtokens/s\n", tokens,
           (double)bytes * iters / secs / 1e6, (double)tokens * iters / secs / 1e6);
    free(src);
    return 0;
}
