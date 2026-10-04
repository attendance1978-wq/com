#ifndef COM_SEMA_ANALYZER_H
#define COM_SEMA_ANALYZER_H

#include "parser/ast.h"
#include "util/error.h"

#define COM_MAX_VARS 256

typedef struct {
    char names[COM_MAX_VARS][64];
    int count;
} SymbolTable;

void symtab_init(SymbolTable *st);
/* slot index, or -1 if unknown */
int symtab_lookup(const SymbolTable *st, const char *name);
/* new slot index, or -1 if the table is full or the name exists */
int symtab_declare(SymbolTable *st, const char *name);

/* Resolves variables to slots, rejecting undefined / redeclared names.
 * Returns 0 on success, 1 on error. */
int analyze(Program *prog, SymbolTable *st, ComError *err);

#endif
