#ifndef COM_PARSER_PARSER_H
#define COM_PARSER_PARSER_H

#include "lexer/lexer.h"
#include "parser/ast.h"
#include "util/arena.h"
#include "util/error.h"

/* Builds the AST in `arena`. Returns 0 on success, 1 on a syntax error. */
int parse(const TokenList *toks, Arena *arena, Program *out, ComError *err);

#endif
