# com

A small programming language written in **base 1**: numbers are tallies of `1`,
so `111` is 3. Written in dependency-free C11, with a stack VM, a C backend and a
base-1 text IR.

```
// examples/01_hello.com
let ten = 1111111111;
let hundred = ten * ten;

printc ten * 1111111 + 11;   // H
printc hundred + 1;          // e
printc hundred + 11111111;   // l
printc hundred + 11111111;   // l
printc hundred + ten + 1;    // o
printc ten;                  // newline
```

## Quick start

```
make
build/com run examples/01_hello.com
build/com repl
make test
```

| command | purpose |
|---------|---------|
| `com run f.com` | run a program (`f.one` also works) |
| `com build f.com --emit c\|one\|ir [-o out]` | compile |
| `com repl` | interactive prompt |
| `comfmt`, `comlint` | formatter and linter |

## The language in one screen

```
let n = 11111;                 // 5
let i = 1 - 1;                 // zero is "1 - 1"
while i < n {
    if i % 11 == 1 - 1 { print i; }
    i = i + 1;
}
```

Variables, `+ - * / %`, `< > == !=`, `while`, `if / else if / else`, `print`,
`printc`. See [docs/syntax.md](docs/syntax.md).

## Layout

```
src/        lexer, parser, sema, ir, codegen, runtime, cli, util
include/    public embedding API (com/api.h)
stdlib/     constant libraries, used with --with
tools/      comfmt, comlint, comrepl
tests/      unit/, integration/, programs/ (end-to-end)
docs/       reference, grammar, stdlib docs, examples
bench/      lexer and parser benchmarks
```

## Status

Version 0.1.0. No functions, strings, arrays or input yet; see
[CHANGELOG.md](CHANGELOG.md). Licensed under MIT. Contributions: see
[CONTRIBUTING.md](CONTRIBUTING.md).
