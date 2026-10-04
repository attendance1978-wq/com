# Getting started

## Build

You need a C11 compiler and `make` (or CMake).

```
make              # build/com, build/comfmt, build/comlint, build/comrepl
make test         # run all tests
```

With CMake: `cmake -S . -B build/cmake && cmake --build build/cmake`.
`scripts/bootstrap.sh` builds, tests and runs hello world in one step.

## Your first program

Save as `hello.com`:

```
let ten = 1111111111;
printc ten * 1111111 + 11;   // 'H' is 72 = 7*10 + 2
printc ten;                  // newline
```

```
build/com run hello.com
```

## Commands

| command | what it does |
|---------|--------------|
| `com run f.com` | compile and run on the VM (`com f.com` is shorthand) |
| `com run f.one` | run base-1 IR text |
| `com build f.com --emit c -o f.c` | C source (then `cc f.c`) |
| `com build f.com --emit one -o f.one` | base-1 IR text |
| `com build f.com --emit ir` | readable IR listing |
| `com repl` | interactive prompt (`:vars`, `:help`, `:quit`) |
| `--with stdlib/io.com` | prepend a library file (repeatable) |
| `--no-opt` | skip constant folding |

Tools: `comfmt [-w] f.com`, `comlint f.com`, `comrepl`.

## Compiling to a native binary

```
build/com build examples/04_c_interop.com --emit c -o fib.c
cc -O2 -o fib fib.c && ./fib
```

## How it works

```
source -> lexer -> parser -> sema -> codegen -> IR -> optimize -> VM
                                                      |-> C backend
                                                      |-> base-1 backend (.one)
```

The IR is a stack machine with 18 opcodes. In the `.one` text format an opcode
is written as (opcode number + 1) ones, one instruction per line, with an
operand after it when needed:

| opcode | number | operand |
|--------|--------|---------|
| nop, add, sub, mul, div, mod, lt, gt, eq, ne, print, printc, halt | see `src/ir/ir.h` | none |
| push | 1 | zigzag: v >= 0 is 2v+1 ones, v < 0 is -2v ones |
| load, store | 2, 3 | slot + 1 ones |
| jmp, jz | 15, 16 | instruction index + 1 ones |

`build/com build f.com --emit one` shows it in practice. Values above 16
million are refused because the unary text would be enormous.
