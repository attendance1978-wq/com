#include "runtime/runtime.h"

#include <stdio.h>

#define STACK_MAX 4096

#define FAIL(...)                                                              \
    do {                                                                       \
        fflush(stdout);                                                        \
        error_set(err, 0, 0, __VA_ARGS__);                                     \
        return 1;                                                              \
    } while (0)

int vm_run(const IR *ir, long *vars, int nvars, ComError *err) {
    long stack[STACK_MAX];
    int sp = 0, pc = 0, n = ir->count;

    while (pc < n) {
        const Instr in = ir->code[pc];
        int at = pc++;
        switch (in.op) {
        case OP_NOP:
            break;
        case OP_PUSH:
            if (sp >= STACK_MAX) FAIL("stack overflow at instruction %d", at);
            stack[sp++] = in.arg;
            break;
        case OP_LOAD:
            if (in.arg < 0 || in.arg >= nvars) FAIL("bad variable slot at instruction %d", at);
            if (sp >= STACK_MAX) FAIL("stack overflow at instruction %d", at);
            stack[sp++] = vars[in.arg];
            break;
        case OP_STORE:
            if (in.arg < 0 || in.arg >= nvars) FAIL("bad variable slot at instruction %d", at);
            if (sp < 1) FAIL("stack underflow at instruction %d", at);
            vars[in.arg] = stack[--sp];
            break;
        case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
        case OP_LT: case OP_GT: case OP_EQ: case OP_NE: {
            long a, b, r;
            if (sp < 2) FAIL("stack underflow at instruction %d", at);
            b = stack[--sp];
            a = stack[--sp];
            if (runtime_binop(in.op, a, b, &r)) FAIL("division by zero at instruction %d", at);
            stack[sp++] = r;
            break;
        }
        case OP_PRINT:
            if (sp < 1) FAIL("stack underflow at instruction %d", at);
            printf("%ld\n", stack[--sp]);
            break;
        case OP_PRINTC:
            if (sp < 1) FAIL("stack underflow at instruction %d", at);
            putchar((int)(unsigned char)stack[--sp]);
            break;
        case OP_JMP:
            if (in.arg < 0 || in.arg > n) FAIL("bad jump target at instruction %d", at);
            pc = (int)in.arg;
            break;
        case OP_JZ: {
            long v;
            if (sp < 1) FAIL("stack underflow at instruction %d", at);
            v = stack[--sp];
            if (v == 0) {
                if (in.arg < 0 || in.arg > n) FAIL("bad jump target at instruction %d", at);
                pc = (int)in.arg;
            }
            break;
        }
        case OP_HALT:
            fflush(stdout);
            return 0;
        default:
            FAIL("bad opcode at instruction %d", at);
        }
    }
    fflush(stdout);
    return 0;
}
