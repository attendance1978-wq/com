// Prints "Hello" - printc takes a character code.
let ten = 1111111111;
let hundred = ten * ten;

printc ten * 1111111 + 11;        // H = 72
printc hundred + 1;              // e = 101
printc hundred + 11111111;         // l = 108
printc hundred + 11111111;         // l = 108
printc hundred + ten + 1;        // o = 111
printc ten;                      // newline
