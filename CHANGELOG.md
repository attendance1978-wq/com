# Changelog

All notable changes to this project are documented here.
Format based on [Keep a Changelog](https://keepachangelog.com/).

## under development version of com
-
-
-
-

## [0.1.0] - 2026-10-04

### Added
- Base-1 (unary) language: numbers are runs of `1`, so `111` is 3. There is no
  `0`, `2`-`9`; zero is written `1 - 1`.
- Statements: `let`, assignment, `print`, `printc`, `while`, `if` / `else if` / `else`, blocks.
- Operators: `+ - * / %`, comparisons `< > == !=`, unary `-`, parentheses.
- Pipeline: lexer, parser (arena-allocated AST), semantic analysis with a
  symbol table, stack-machine IR, constant folding.
- Backends: stack VM, C source emitter, base-1 text IR (`.one`) with a loader.
- CLI: `com run`, `com build --emit c|one|ir`, `com repl`, `--with` prelude files.
- Tools: `comfmt` (formatter), `comlint` (errors + unused variables), `comrepl`.
- Unit, integration and end-to-end tests; lexer/parser benchmarks.
- Makefile and CMake builds, docs, example programs, stdlib constants.

### Known limitations
- No functions, strings, arrays or input yet; the stdlib files are constants only.
- All variables live in one flat scope (max 256).
- No bundled editor extension (VS Code highlighting) yet.
