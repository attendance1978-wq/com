#include "test.h"
#include "com/com.h"
#include "runtime/runtime.h"

#include <stdlib.h>
#include <string.h>

int main(void) {
    /* public API: compile to C */
    char *c = com_compile_to_c("let x = 11; print x * 111;", "<test>");
    CHECK(c != NULL);
    CHECK(c && strstr(c, "int main(void)") != NULL);
    CHECK(c && strstr(c, "printf") != NULL);
    free(c);

    /* public API: base-1 output contains only 1s and whitespace */
    char *one = com_compile_to_one("let x = 11; print x;", "<test>");
    CHECK(one != NULL);
    CHECK(one && strspn(one, "1 \n") == strlen(one));
    free(one);

    /* readable IR listing */
    char *ir = com_emit("print 1;", "<test>", EMIT_IR, 1);
    CHECK(ir && strstr(ir, "push") != NULL && strstr(ir, "print") != NULL && strstr(ir, "halt") != NULL);
    free(ir);

    /* errors give NULL (message goes to stderr) */
    CHECK(com_compile_to_c("print nope;", "<test>") == NULL);
    CHECK(com_compile_to_one("print 12;", "<test>") == NULL);

    /* running through the public API */
    CHECK(com_run_source("let q = 1;", "<test>") == 0);
    CHECK(com_run_source("let q = ;", "<test>") == 1);

    /* version header */
    CHECK(strcmp(COM_VERSION_STRING, "0.1.0") == 0);

    TEST_DONE();
}
