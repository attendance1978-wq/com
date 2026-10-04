# io

`stdlib/io.com` - character codes for `printc`.

| name | value | character |
|------|-------|-----------|
| `NL` | 10 | newline |
| `SPACE` | 32 | space |
| `DIGIT0` | 48 | `'0'` (add a digit 0-9 to print it) |

```
printc DIGIT0 + 111;   // prints 3
printc NL;
```

Run with `com run --with stdlib/io.com prog.com`. `stdlib/string.com` adds `CH_A`
(65), `CH_a` (97) and `CH_BANG` (33) the same way.
