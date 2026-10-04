# core

`stdlib/core.com` - boolean constants.

| name | value | meaning |
|------|-------|---------|
| `FALSE` | 0 | false |
| `TRUE`  | 1 | true  |

```
com run --with stdlib/core.com prog.com
```

## About the standard library

com has no functions or imports yet, so the stdlib is a set of constants that
`--with` prepends to your program as plain text. Consequences:

- every constant uses one variable slot (limit 256) and cannot be redeclared, so
  do not `let` a name that a prelude already defines;
- line numbers in error messages count the prelude lines too.
