# Contributing

## Build and test

```
make            # builds build/com and the tools
make test       # unit + integration tests, then tests/run_tests.sh
make bench      # lexer / parser benchmarks
```

Or with CMake: `cmake -S . -B build/cmake && cmake --build build/cmake && ctest --test-dir build/cmake`.

Before sending a change, also run the tests under sanitizers:

```
make clean
make CFLAGS="-std=c11 -Wall -Wextra -g -fsanitize=address,undefined" test
```

## Code layout

| directory        | purpose                                                    |
|------------------|------------------------------------------------------------|
| `src/lexer`      | source text -> tokens                                      |
| `src/parser`     | tokens -> AST (`ast.h`)                                    |
| `src/sema`       | name resolution, symbol table                              |
| `src/ir`         | stack IR, optimiser                                        |
| `src/codegen`    | AST -> IR, plus the C and base-1 backends                  |
| `src/runtime`    | VM, shared arithmetic, compile/run entry points            |
| `src/cli`        | command line and REPL                                      |
| `src/util`       | arena, string buffer, error reporting                      |
| `include/com`    | public embedding API                                       |

## Guidelines

- C11, no compiler extensions, no warnings under `-Wall -Wextra`.
- Includes are relative to `src/` (`#include "lexer/lexer.h"`). Do not add
  `src/util` to the include path: it contains a `string.h`.
- New language features need: a grammar change in `docs/grammar.ebnf`, a note in
  `docs/syntax.md`, unit tests, and a program in `tests/programs/pass` (with a
  `.out` file) or `tests/programs/fail`.
- If you add an opcode, update the VM, the C backend, the base-1 loader and
  `docs/getting-started.md`.
- Keep `comfmt` idempotent: formatting formatted code must not change it.
