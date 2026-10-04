#ifndef COM_LEXER_TOKEN_H
#define COM_LEXER_TOKEN_H

typedef enum {
    TOK_EOF,
    TOK_NUM,     /* a run of 1s: base 1, value = number of 1s */
    TOK_IDENT,
    TOK_COMMENT, /* only produced when comments are requested */
    TOK_LET,
    TOK_PRINT,
    TOK_PRINTC,
    TOK_WHILE,
    TOK_IF,
    TOK_ELSE,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_LT,
    TOK_GT,
    TOK_EQEQ,
    TOK_NEQ,
    TOK_ASSIGN,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMI,
    TOK_COUNT_
} TokenType;

typedef struct {
    TokenType type;
    int line;
    int col;
    long value;        /* TOK_NUM */
    const char *start; /* points into the source text */
    int len;
} Token;

/* "';'", "identifier", ... for diagnostics */
const char *token_name(TokenType t);
/* raw source text of keywords/symbols ("let", "+"), NULL for the rest */
const char *token_text(TokenType t);

#endif
