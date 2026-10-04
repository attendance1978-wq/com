# Syntax

## Numbers are written in base 1

A number is a run of the digit `1`. Its value is how many `1`s there are.

| written  | value |
|----------|-------|
| `1`      | 1     |
| `111`    | 3     |
| `1111111111` | 10 |

There is no `0` and no digit `2`-`9`; writing one is a lexer error
(`invalid digit '2': base 1 has only the digit '1'`). Zero is `1 - 1`.
Negative numbers come from subtraction or unary minus: `-111` is -3.

A number cannot touch a letter: `1a` is an error. Names may contain digits
after the first character (`x1`, `v2`).

## Statements

```
let x = 111;          // declare a variable (must not already exist)
x = x + 1;            // assign (variable must exist)
print x;              // print a number and a newline
printc 1111111111;    // print a character by code (10 = newline)

while x > 1 {         // loop while the condition is not zero
    x = x - 1;
}

if x == 1 {           // 0 is false, anything else is true
    print 111;
} else if x == 11 {
    print 1111;
} else {
    print 11111;
}

{ let tmp = x * 11; } // bare block
```

Every statement except `while`, `if` and blocks ends with `;`.

## Expressions

Arithmetic `+ - * / %` on signed 64-bit integers (overflow wraps; `/` and `%`
truncate toward zero; dividing by zero is a runtime error).
Comparisons `< > == !=` give `1` or `0`. Precedence, high to low:

1. unary `-`, parentheses
2. `* / %`
3. `+ -`
4. `< > == !=`

All binary operators are left-associative. See `grammar.ebnf` for the formal grammar.

## Scope

There is one flat scope: a variable declared inside a block is still visible
after it. A name can be declared only once, and at most 256 variables exist.

## Comments

`// ...` to the end of the line.

## Errors

Every error is reported as `file:line:col: error: message`; runtime errors
(division by zero) have no position because they come from the IR.
