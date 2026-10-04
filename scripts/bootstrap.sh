#!/bin/sh
# One-shot setup check for a fresh checkout: toolchain, build, tests, hello world.
set -e
cd "$(dirname "$0")/.."

for tool in cc make; do
    command -v "$tool" >/dev/null 2>&1 || { echo "missing required tool: $tool" >&2; exit 1; }
done
command -v cmake >/dev/null 2>&1 || echo "note: cmake not found (optional, the Makefile is enough)"

mkdir -p build
sh scripts/build.sh
echo
echo "--- examples/01_hello.com"
build/com run examples/01_hello.com
echo
echo "Ready. Try: build/com repl"
