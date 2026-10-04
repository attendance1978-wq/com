#!/bin/sh
# End-to-end tests over tests/programs. For every passing program the output of
# the VM, the C backend and the base-1 (.one) backend must all match the .out file.
# Usage: COM=build/com sh tests/run_tests.sh

COM=${COM:-build/com}
DIR=$(dirname "$0")/programs
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

pass=0
fail=0
ok()  { pass=$((pass + 1)); }
bad() { fail=$((fail + 1)); echo "FAIL: $1"; }

have_cc=0
command -v cc >/dev/null 2>&1 && have_cc=1

for f in "$DIR"/pass/*.com; do
    name=$(basename "$f" .com)
    exp="$DIR/pass/$name.out"

    "$COM" run "$f" >"$TMP/vm.out" 2>"$TMP/vm.err"
    if [ $? -eq 0 ] && cmp -s "$TMP/vm.out" "$exp"; then ok; else bad "$name (vm)"; fi

    "$COM" run --no-opt "$f" >"$TMP/noopt.out" 2>/dev/null
    if cmp -s "$TMP/noopt.out" "$exp"; then ok; else bad "$name (--no-opt)"; fi

    "$COM" build "$f" --emit one -o "$TMP/p.one" 2>/dev/null &&
        "$COM" run "$TMP/p.one" >"$TMP/one.out" 2>/dev/null
    if cmp -s "$TMP/one.out" "$exp"; then ok; else bad "$name (.one)"; fi

    if [ $have_cc -eq 1 ]; then
        "$COM" build "$f" --emit c -o "$TMP/p.c" 2>/dev/null &&
            cc -w -o "$TMP/p" "$TMP/p.c" 2>/dev/null &&
            "$TMP/p" >"$TMP/c.out" 2>/dev/null
        if cmp -s "$TMP/c.out" "$exp"; then ok; else bad "$name (C backend)"; fi
    fi
done

for f in "$DIR"/fail/*.com; do
    name=$(basename "$f" .com)
    "$COM" run "$f" >/dev/null 2>"$TMP/err"
    rc=$?
    if [ $rc -ne 0 ] && [ -s "$TMP/err" ]; then ok; else bad "$name (should fail with a message)"; fi
done

# stdlib prelude
printf 'print NL;\n' >"$TMP/prelude.com"
if [ "$("$COM" run --with stdlib/io.com "$TMP/prelude.com" 2>/dev/null)" = "10" ]; then ok; else bad "--with stdlib/io.com"; fi

echo "programs: $pass passed, $fail failed"
[ $fail -eq 0 ]
