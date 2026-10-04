#ifndef COM_LEXER_LEXER_H
#define COM_LEXER_LEXER_H

#include "lexer/token.h"
#include "util/error.h"

typedef struct {
    Token *toks;
    int count;
    int cap;
} TokenList;

/* Tokenises `src` (which must outlive the tokens). The list always ends with
 * TOK_EOF. Returns 0 on success, 1 on error (list is then empty). */
int lex(const char *src, TokenList *out, ComError *err, int keep_comments);
void token_list_free(TokenList *l);

#endif
