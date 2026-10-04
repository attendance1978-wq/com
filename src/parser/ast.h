#ifndef COM_PARSER_AST_H
#define COM_PARSER_AST_H

typedef enum { EXPR_NUM, EXPR_VAR, EXPR_BIN } ExprKind;

/* Order matches OP_ADD..OP_NE in ir.h; codegen relies on it. */
typedef enum {
    BIN_ADD, BIN_SUB, BIN_MUL, BIN_DIV, BIN_MOD,
    BIN_LT, BIN_GT, BIN_EQ, BIN_NE
} BinOp;

typedef struct Expr Expr;
struct Expr {
    ExprKind kind;
    int line, col;
    long num;       /* EXPR_NUM */
    char name[64];  /* EXPR_VAR */
    int slot;       /* EXPR_VAR, filled in by sema */
    BinOp op;       /* EXPR_BIN */
    Expr *lhs, *rhs;
};

typedef enum {
    STMT_LET, STMT_ASSIGN, STMT_PRINT, STMT_PRINTC,
    STMT_WHILE, STMT_IF, STMT_BLOCK
} StmtKind;

typedef struct Stmt Stmt;
struct Stmt {
    StmtKind kind;
    int line, col;
    char name[64];   /* LET / ASSIGN */
    int slot;        /* LET / ASSIGN, filled in by sema */
    Expr *expr;      /* LET/ASSIGN value, PRINT/PRINTC value, WHILE/IF condition */
    Stmt *body;      /* BLOCK: first statement; WHILE/IF: a BLOCK */
    Stmt *else_body; /* IF: a BLOCK or NULL */
    Stmt *next;
};

typedef struct {
    Stmt *first;
} Program;

#endif
