#include "sema/analyzer.h"

#include <string.h>

void symtab_init(SymbolTable *st) { st->count = 0; }

int symtab_lookup(const SymbolTable *st, const char *name) {
    for (int i = 0; i < st->count; i++)
        if (strcmp(st->names[i], name) == 0) return i;
    return -1;
}

int symtab_declare(SymbolTable *st, const char *name) {
    if (st->count >= COM_MAX_VARS || symtab_lookup(st, name) >= 0) return -1;
    strncpy(st->names[st->count], name, 63);
    st->names[st->count][63] = '\0';
    return st->count++;
}
