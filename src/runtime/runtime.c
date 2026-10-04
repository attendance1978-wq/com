#include "runtime/runtime.h"

#include "codegen/codegen.h"
#include "com/api.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "util/arena.h"

#include <stdio.h>
#include <stdlib.h>

int runtime_binop(OpCode op, long a, long b, long *out) {
    unsigned long ua = (unsigned long)a, ub = (unsigned long)b;
    switch (op) {
    case OP_ADD: *out = (long)(ua + ub); return 0;
    case OP_SUB: *out = (long)(ua - ub); return 0;
    case OP_MUL: *out = (long)(ua * ub); return 0;
    case OP_DIV:
        if (b == 0) return 1;
        *out = (b == -1) ? (long)(0UL - ua) : a / b;
        return 0;
    case OP_MOD:
        if (b == 0) return 1;
        *out = (b == -1) ? 0 : a % b;
        return 0;
    case OP_LT: *out = a < b; return 0;
    case OP_GT: *out = a > b; return 0;
    case OP_EQ: *out = a == b; return 0;
    case OP_NE: *out = a != b; return 0;
    default: return 1;
    }
}

int com_compile(const char *src, IR *ir, SymbolTable *syms, ComError *err, int opt) {
    TokenList tl;
    Arena arena;
    Program prog;
    int rc = 1;

    ir_init(ir);
    if (lex(src, &tl, err, 0)) return 1;
    arena_init(&arena);
    if (parse(&tl, &arena, &prog, err) == 0 && analyze(&prog, syms, err) == 0) {
        codegen(&prog, ir);
        if (opt) optimize(ir);
        rc = 0;
    }
    arena_free(&arena);
    token_list_free(&tl);
    return rc;
}

int com_run_source_opt(const char *src, const char *filename, int opt) {
    SymbolTable st;
    IR ir;
    ComError err;
    long vars[COM_MAX_VARS] = {0};

    symtab_init(&st);
    if (com_compile(src, &ir, &st, &err, opt)) {
        error_print(&err, filename);
        ir_free(&ir);
        return 1;
    }
    int rc = vm_run(&ir, vars, COM_MAX_VARS, &err);
    if (rc) error_print(&err, filename);
    ir_free(&ir);
    return rc;
}

char *com_emit(const char *src, const char *filename, EmitKind kind, int opt) {
    SymbolTable st;
    IR ir;
    ComError err;
    char *out = NULL;

    symtab_init(&st);
    if (com_compile(src, &ir, &st, &err, opt)) {
        error_print(&err, filename);
        ir_free(&ir);
        return NULL;
    }
    switch (kind) {
    case EMIT_C:
        out = emit_c(&ir, st.count);
        break;
    case EMIT_ONE:
        out = emit_one(&ir);
        if (!out) fprintf(stderr, "%s: error: a value is too large to write in base 1\n", filename);
        break;
    case EMIT_IR:
        out = ir_to_string(&ir);
        break;
    }
    ir_free(&ir);
    return out;
}

/* ---- public API (include/com/api.h) ---- */

int com_run_source(const char *src, const char *filename) {
    return com_run_source_opt(src, filename, 1);
}
char *com_compile_to_c(const char *src, const char *filename) {
    return com_emit(src, filename, EMIT_C, 1);
}
char *com_compile_to_one(const char *src, const char *filename) {
    return com_emit(src, filename, EMIT_ONE, 1);
}
