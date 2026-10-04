#include "codegen/codegen.h"

static void gen_stmts(IR *ir, const Stmt *s);

static void gen_expr(IR *ir, const Expr *e) {
    switch (e->kind) {
    case EXPR_NUM:
        ir_emit(ir, OP_PUSH, e->num);
        break;
    case EXPR_VAR:
        ir_emit(ir, OP_LOAD, e->slot);
        break;
    case EXPR_BIN:
        gen_expr(ir, e->lhs);
        gen_expr(ir, e->rhs);
        ir_emit(ir, (OpCode)(OP_ADD + (int)e->op), 0);
        break;
    }
}

static void gen_stmt(IR *ir, const Stmt *s) {
    switch (s->kind) {
    case STMT_LET:
    case STMT_ASSIGN:
        gen_expr(ir, s->expr);
        ir_emit(ir, OP_STORE, s->slot);
        break;
    case STMT_PRINT:
        gen_expr(ir, s->expr);
        ir_emit(ir, OP_PRINT, 0);
        break;
    case STMT_PRINTC:
        gen_expr(ir, s->expr);
        ir_emit(ir, OP_PRINTC, 0);
        break;
    case STMT_BLOCK:
        gen_stmts(ir, s->body);
        break;
    case STMT_WHILE: {
        int start = ir->count;
        gen_expr(ir, s->expr);
        int jz = ir_emit(ir, OP_JZ, 0);
        gen_stmt(ir, s->body);
        ir_emit(ir, OP_JMP, start);
        ir->code[jz].arg = ir->count;
        break;
    }
    case STMT_IF: {
        gen_expr(ir, s->expr);
        int jz = ir_emit(ir, OP_JZ, 0);
        gen_stmt(ir, s->body);
        if (s->else_body) {
            int jmp = ir_emit(ir, OP_JMP, 0);
            ir->code[jz].arg = ir->count;
            gen_stmt(ir, s->else_body);
            ir->code[jmp].arg = ir->count;
        } else {
            ir->code[jz].arg = ir->count;
        }
        break;
    }
    }
}

static void gen_stmts(IR *ir, const Stmt *s) {
    for (; s; s = s->next) gen_stmt(ir, s);
}

int codegen(const Program *prog, IR *ir) {
    gen_stmts(ir, prog->first);
    ir_emit(ir, OP_HALT, 0);
    return 0;
}
