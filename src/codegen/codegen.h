#ifndef COM_CODEGEN_CODEGEN_H
#define COM_CODEGEN_CODEGEN_H

#include "ir/ir.h"
#include "parser/ast.h"
#include "util/error.h"

/* AST -> IR. `ir` must be initialised. Returns 0. */
int codegen(const Program *prog, IR *ir);

/* Backends. Both return malloc'd text (caller frees). */
char *emit_c(const IR *ir, int nvars);
/* NULL if a value is too large to write in unary. */
char *emit_one(const IR *ir);

/* Loads the base-1 text format back into IR. `ir` is (re)initialised.
 * Returns 0 on success, 1 on error. */
int one_load(const char *text, IR *ir, ComError *err);

#endif
