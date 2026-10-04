#include "ir/ir.h"
#include "util/error.h"
#include "util/string.h"

#include <stdlib.h>

static const char *const names[OP_COUNT] = {
    [OP_NOP] = "nop",   [OP_PUSH] = "push", [OP_LOAD] = "load", [OP_STORE] = "store",
    [OP_ADD] = "add",   [OP_SUB] = "sub",   [OP_MUL] = "mul",   [OP_DIV] = "div",
    [OP_MOD] = "mod",   [OP_LT] = "lt",     [OP_GT] = "gt",     [OP_EQ] = "eq",
    [OP_NE] = "ne",     [OP_PRINT] = "print", [OP_PRINTC] = "printc",
    [OP_JMP] = "jmp",   [OP_JZ] = "jz",     [OP_HALT] = "halt",
};

void ir_init(IR *ir) {
    ir->code = NULL;
    ir->count = 0;
    ir->cap = 0;
}

void ir_free(IR *ir) {
    free(ir->code);
    ir_init(ir);
}

int ir_emit(IR *ir, OpCode op, long arg) {
    if (ir->count == ir->cap) {
        int nc = ir->cap ? ir->cap * 2 : 64;
        Instr *n = realloc(ir->code, (size_t)nc * sizeof(Instr));
        if (!n) error_fatal("out of memory");
        ir->code = n;
        ir->cap = nc;
    }
    ir->code[ir->count].op = op;
    ir->code[ir->count].arg = arg;
    return ir->count++;
}

const char *op_name(OpCode op) { return (op >= 0 && op < OP_COUNT) ? names[op] : "?"; }

int op_has_arg(OpCode op) {
    return op == OP_PUSH || op == OP_LOAD || op == OP_STORE || op == OP_JMP || op == OP_JZ;
}

char *ir_to_string(const IR *ir) {
    StrBuf sb;
    sb_init(&sb);
    for (int i = 0; i < ir->count; i++) {
        sb_printf(&sb, "%4d  %-6s", i, op_name(ir->code[i].op));
        if (op_has_arg(ir->code[i].op)) sb_printf(&sb, " %ld", ir->code[i].arg);
        sb_putc(&sb, '\n');
    }
    return sb_take(&sb);
}

void ir_dump(const IR *ir, FILE *f) {
    char *s = ir_to_string(ir);
    fputs(s, f);
    free(s);
}
