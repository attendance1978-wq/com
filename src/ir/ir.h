#ifndef COM_IR_IR_H
#define COM_IR_IR_H

#include <stdio.h>

/* Stack-machine IR. OP_ADD..OP_NE must stay contiguous and in the same order
 * as BIN_ADD..BIN_NE in ast.h. In the base-1 text format an opcode is written
 * as (opcode + 1) ones, since zero cannot be written in unary. */
typedef enum {
    OP_NOP, OP_PUSH, OP_LOAD, OP_STORE,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_LT, OP_GT, OP_EQ, OP_NE,
    OP_PRINT, OP_PRINTC,
    OP_JMP, OP_JZ, OP_HALT,
    OP_COUNT
} OpCode;

typedef struct {
    OpCode op;
    long arg;
} Instr;

typedef struct {
    Instr *code;
    int count;
    int cap;
} IR;

void ir_init(IR *ir);
void ir_free(IR *ir);
int ir_emit(IR *ir, OpCode op, long arg); /* returns the instruction index */
const char *op_name(OpCode op);
int op_has_arg(OpCode op);
char *ir_to_string(const IR *ir); /* human-readable listing, caller frees */
void ir_dump(const IR *ir, FILE *f);

/* constant folding + NOP removal (jump targets are remapped) */
void optimize(IR *ir);

#endif
