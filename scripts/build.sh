#!/bin/sh
# Build com, the tools and run the test-suite.   Usage: scripts/build.sh [--no-test]
set -e
cd "$(dirname "$0")/.."
make -j"$(nproc 2>/dev/null || echo 2)" all
[ "$1" = "--no-test" ] || make test
echo "built: build/com"
