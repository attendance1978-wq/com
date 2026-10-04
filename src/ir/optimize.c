#include "ir/ir.h"
#include "runtime/runtime.h"

#include <stdlib.h>

static int is_binop(OpCode op) { return op >= OP_ADD && op <= OP_NE; }
static int is_jump(OpCode op) { return op == OP_JMP || op == OP_JZ; }

/* push a; push b; <binop>  ->  push (a op b); nop; nop */
static int fold_pass(IR *ir) {
    int n = ir->count, changed = 0;
    char *target = calloc((size_t)n + 2, 1);
    if (!target) return 0;
    for (int i = 0; i < n; i++)
        if (is_jump(ir->code[i].op) && ir->code[i].arg >= 0 && ir->code[i].arg <= n)
            target[ir->code[i].arg] = 1;

    for (int i = 0; i + 2 < n; i++) {
        Instr *a = &ir->code[i], *b = &ir->code[i + 1], *o = &ir->code[i + 2];
        if (a->op == OP_PUSH && b->op == OP_PUSH && is_binop(o->op) && !target[i + 1] && !target[i + 2]) {
            long r;
            if (runtime_binop(o->op, a->arg, b->arg, &r) != 0) continue; /* keep div-by-zero for runtime */
            a->arg = r;
            b->op = OP_NOP;
            o->op = OP_NOP;
            changed = 1;
            i += 2;
        }
    }
    free(target);
    return changed;
}

static void compact(IR *ir) {
    int n = ir->count, k = 0;
    int *map = malloc(((size_t)n + 1) * sizeof(int));
    if (!map) return;
    for (int i = 0; i < n; i++) {
        map[i] = k;
        if (ir->code[i].op != OP_NOP) k++;
    }
    map[n] = k;
    int j = 0;
    for (int i = 0; i < n; i++)
        if (ir->code[i].op != OP_NOP) ir->code[j++] = ir->code[i];
    for (int i = 0; i < k; i++)
        if (is_jump(ir->code[i].op) && ir->code[i].arg >= 0 && ir->code[i].arg <= n)
            ir->code[i].arg = map[ir->code[i].arg];
    ir->count = k;
    free(map);
}

void optimize(IR *ir) {
    while (fold_pass(ir)) compact(ir);
}
