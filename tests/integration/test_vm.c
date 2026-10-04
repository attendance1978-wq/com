#include "test.h"
#include "runtime/runtime.h"

#include <string.h>

/* Compiles and runs `src`, leaving variable values in `vars`. */
static int run(const char *src, long *vars, int opt) {
    SymbolTable st;
    IR ir;
    ComError err;
    int rc;
    symtab_init(&st);
    rc = com_compile(src, &ir, &st, &err, opt);
    if (rc == 0) rc = vm_run(&ir, vars, COM_MAX_VARS, &err);
    ir_free(&ir);
    return rc;
}

int main(void) {
    for (int opt = 0; opt <= 1; opt++) { /* results must not depend on the optimiser */
        long v[COM_MAX_VARS];

        memset(v, 0, sizeof v);
        CHECK(run("let x = 1 + 11 * 111;", v, opt) == 0 && v[0] == 7);

        memset(v, 0, sizeof v);
        CHECK(run("let y = -111;", v, opt) == 0 && v[0] == -3);

        memset(v, 0, sizeof v);
        CHECK(run("let s = 1 - 1; let i = 1; while i < 11111 { s = s + i; i = i + 1; }", v, opt) == 0);
        CHECK(v[0] == 10 && v[1] == 5);

        memset(v, 0, sizeof v);
        CHECK(run("let r = 1 - 1; if 11 > 1 { r = 111; } else { r = 1111; }", v, opt) == 0 && v[0] == 3);

        memset(v, 0, sizeof v);
        CHECK(run("let r = 1 - 1; if 1 > 11 { r = 111; } else if 1 == 1 { r = 11; } else { r = 1; }", v, opt) == 0 && v[0] == 2);

        memset(v, 0, sizeof v);
        CHECK(run("let a = 1 - 1; let b = 1 / a;", v, opt) == 1); /* division by zero is a runtime error */

        memset(v, 0, sizeof v);
        CHECK(run("let m = 11111 % 111;", v, opt) == 0 && v[0] == 2);
    }
    TEST_DONE();
}
