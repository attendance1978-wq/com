# Benchmark results

Run with `make bench`. Numbers are from a single run on a 1-core x86_64 container
(gcc 13, `-O2`); expect different numbers on your machine.

| benchmark      | what it measures                                   | result                          |
|----------------|----------------------------------------------------|---------------------------------|
| `bench_lexer`  | tokenising 20k lines (500k tokens), 20 iterations  | ~254 MB/s, ~92 Mtokens/s        |
| `bench_parser` | parsing 500k tokens into an AST, 20 iterations     | ~33 Mtokens/s                   |

Both use a synthetic program, so treat them as a regression guide rather than
a statement about real-world programs. There is no VM benchmark yet.
