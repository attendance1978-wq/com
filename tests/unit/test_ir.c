#include "test.h"
#include "codegen/codegen.h"
#include "ir/ir.h"
#include "runtime/runtime.h"

#include <stdlib.h>
#include <string.h>

int main(void) {
    IR ir;
    long r;

    /* constant folding */
    ir_init(&ir);
    ir_emit(&ir, OP_PUSH, 2);
    ir_emit(&ir, OP_PUSH, 3);
    ir_emit(&ir, OP_ADD, 0);
    ir_emit(&ir, OP_PRINT, 0);
    ir_emit(&ir, OP_HALT, 0);
    optimize(&ir);
    CHECK(ir.count == 3);
    CHECK(ir.code[0].op == OP_PUSH && ir.code[0].arg == 5);
    ir_free(&ir);

    /* chained folds: (1 + 1) + 1 */
    ir_init(&ir);
    ir_emit(&ir, OP_PUSH, 1);
    ir_emit(&ir, OP_PUSH, 1);
    ir_emit(&ir, OP_ADD, 0);
    ir_emit(&ir, OP_PUSH, 1);
    ir_emit(&ir, OP_ADD, 0);
    optimize(&ir);
    CHECK(ir.count == 1 && ir.code[0].arg == 3);
    ir_free(&ir);

    /* jump targets are remapped after NOPs disappear */
    ir_init(&ir);
    ir_emit(&ir, OP_PUSH, 1);   /* 0 */
    ir_emit(&ir, OP_PUSH, 1);   /* 1 */
    ir_emit(&ir, OP_ADD, 0);    /* 2 */
    ir_emit(&ir, OP_STORE, 0);  /* 3 */
    ir_emit(&ir, OP_PUSH, 0);   /* 4 */
    ir_emit(&ir, OP_JZ, 7);     /* 5 */
    ir_emit(&ir, OP_JMP, 0);    /* 6 */
    ir_emit(&ir, OP_HALT, 0);   /* 7 */
    optimize(&ir);
    CHECK(ir.count == 6);
    CHECK(ir.code[3].op == OP_JZ && ir.code[3].arg == 5);
    CHECK(ir.code[4].op == OP_JMP && ir.code[4].arg == 0);
    ir_free(&ir);

    /* division by zero is never folded away */
    ir_init(&ir);
    ir_emit(&ir, OP_PUSH, 1);
    ir_emit(&ir, OP_PUSH, 0);
    ir_emit(&ir, OP_DIV, 0);
    optimize(&ir);
    CHECK(ir.count == 3);
    ir_free(&ir);

    /* arithmetic helpers */
    CHECK(runtime_binop(OP_DIV, 7, 2, &r) == 0 && r == 3);
    CHECK(runtime_binop(OP_MOD, 7, 2, &r) == 0 && r == 1);
    CHECK(runtime_binop(OP_DIV, 1, 0, &r) == 1);
    CHECK(runtime_binop(OP_LT, 1, 2, &r) == 0 && r == 1);

    /* base-1 round trip, including negative numbers and jumps */
    {
        const char *src = "let x = 1; while x < 111 { x = x + 1; } print 1 - 11 * x;";
        SymbolTable st;
        ComError err;
        IR a, b;
        symtab_init(&st);
        CHECK(com_compile(src, &a, &st, &err, 1) == 0);
        char *text = emit_one(&a);
        CHECK(text != NULL);
        CHECK(strspn(text, "1 \n") == strlen(text)); /* only 1s and whitespace */
        CHECK(one_load(text, &b, &err) == 0);
        CHECK(a.count == b.count);
        for (int i = 0; i < a.count && i < b.count; i++)
            CHECK(a.code[i].op == b.code[i].op && a.code[i].arg == b.code[i].arg);
        free(text);
        ir_free(&a);
        ir_free(&b);
    }

    /* loader rejects bad input */
    {
        ComError err;
        IR b;
        CHECK(one_load("1x", &b, &err) == 1);
        ir_free(&b);
        CHECK(one_load("11", &b, &err) == 1); /* push without an operand */
        ir_free(&b);
        CHECK(one_load("1", &b, &err) == 0);  /* nop */
        CHECK(b.count == 1 && b.code[0].op == OP_NOP);
        ir_free(&b);
        CHECK(one_load("11111111111111111111111111111", &b, &err) == 1); /* unknown opcode */
        ir_free(&b);
        CHECK(one_load("1111111111111111 111", &b, &err) == 1); /* jmp past the end */
        ir_free(&b);
    }

    TEST_DONE();
}
