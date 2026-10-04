#ifndef COM_RUNTIME_RUNTIME_H
#define COM_RUNTIME_RUNTIME_H

#include "ir/ir.h"
#include "sema/analyzer.h"
#include "util/error.h"

typedef enum { EMIT_C, EMIT_ONE, EMIT_IR } EmitKind;

/* Shared arithmetic for the VM and the constant folder.
 * Returns 0 on success, 1 on division by zero. Arithmetic wraps. */
int runtime_binop(OpCode op, long a, long b, long *out);

/* Executes `ir`. `vars` must have room for `nvars` slots. 0 on success. */
int vm_run(const IR *ir, long *vars, int nvars, ComError *err);

/* lex + parse + analyse + codegen (+ optimise). `ir` is always initialised and
 * must be freed by the caller. `syms` may already hold declarations (REPL). */
int com_compile(const char *src, IR *ir, SymbolTable *syms, ComError *err, int opt);

int com_run_source_opt(const char *src, const char *filename, int opt);
char *com_emit(const char *src, const char *filename, EmitKind kind, int opt);

#endif
