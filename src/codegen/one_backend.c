#include "codegen/codegen.h"
#include "util/string.h"

#include <ctype.h>

/* Base-1 text format
 *   instruction := ones(opcode+1) [ ' ' ones(operand) ] '\n'
 *   PUSH operand : zigzag  (v >= 0 -> 2v+1 ones, v < 0 -> -2v ones)
 *   other operands (slot / jump target) : n+1 ones
 * Everything is written with the single digit 1 and whitespace. */

#define ONE_LIMIT (1UL << 24) /* refuse to write absurdly long tallies */

static int put_ones(StrBuf *sb, unsigned long n) {
    if (n > ONE_LIMIT) return 1;
    for (unsigned long i = 0; i < n; i++) sb_putc(sb, '1');
    return 0;
}

static int zigzag(long v, unsigned long *out) {
    unsigned long mag = v < 0 ? 0UL - (unsigned long)v : (unsigned long)v;
    if (mag > ONE_LIMIT) return 1;
    *out = v >= 0 ? 2 * mag + 1 : 2 * mag;
    return 0;
}

char *emit_one(const IR *ir) {
    StrBuf sb;
    sb_init(&sb);
    for (int i = 0; i < ir->count; i++) {
        const Instr *in = &ir->code[i];
        put_ones(&sb, (unsigned long)in->op + 1);
        if (op_has_arg(in->op)) {
            unsigned long enc;
            sb_putc(&sb, ' ');
            if (in->op == OP_PUSH) {
                if (zigzag(in->arg, &enc) || put_ones(&sb, enc)) { sb_free(&sb); return NULL; }
            } else {
                if (in->arg < 0 || put_ones(&sb, (unsigned long)in->arg + 1)) { sb_free(&sb); return NULL; }
            }
        }
        sb_putc(&sb, '\n');
    }
    return sb_take(&sb);
}

/* returns 0 ok, 1 error, 2 end of input */
static int read_run(const char **pp, int *line, unsigned long *out, ComError *err) {
    const char *p = *pp;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
        if (*p == '\n') (*line)++;
        p++;
    }
    if (!*p) { *pp = p; return 2; }
    if (*p != '1') {
        error_set(err, *line, 0, "unexpected character '%c' (a .one file contains only 1s and whitespace)", *p);
        return 1;
    }
    unsigned long n = 0;
    while (*p == '1') { n++; p++; }
    if (*p && !isspace((unsigned char)*p)) {
        error_set(err, *line, 0, "unexpected character '%c' (a .one file contains only 1s and whitespace)", *p);
        return 1;
    }
    *pp = p;
    *out = n;
    return 0;
}

int one_load(const char *text, IR *ir, ComError *err) {
    ir_init(ir);
    const char *p = text;
    int line = 1;
    for (;;) {
        unsigned long n;
        int rc = read_run(&p, &line, &n, err);
        if (rc == 2) break;
        if (rc == 1) return 1;
        if (n - 1 >= (unsigned long)OP_COUNT) {
            error_set(err, line, 0, "unknown opcode %lu", n);
            return 1;
        }
        OpCode op = (OpCode)(n - 1);
        long arg = 0;
        if (op_has_arg(op)) {
            unsigned long m;
            int rc2 = read_run(&p, &line, &m, err);
            if (rc2 == 2) {
                error_set(err, line, 0, "missing operand for '%s'", op_name(op));
                return 1;
            }
            if (rc2 == 1) return 1;
            if (op == OP_PUSH) arg = (m & 1) ? (long)((m - 1) / 2) : -(long)(m / 2);
            else arg = (long)(m - 1);
        }
        ir_emit(ir, op, arg);
    }
    for (int i = 0; i < ir->count; i++) {
        OpCode op = ir->code[i].op;
        long a = ir->code[i].arg;
        if ((op == OP_JMP || op == OP_JZ) && (a < 0 || a > ir->count)) {
            error_set(err, 0, 0, "instruction %d: jump target %ld out of range", i, a);
            return 1;
        }
        if ((op == OP_LOAD || op == OP_STORE) && (a < 0 || a >= 256)) {
            error_set(err, 0, 0, "instruction %d: variable slot %ld out of range", i, a);
            return 1;
        }
    }
    return 0;
}
