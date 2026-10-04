// Prints 42 in binary: 101010
let ten = 1111111111;
let n = ten * 1111 + 11;          // 42
let p = ten * 111 + 11;           // 32, the highest power of two <= n
let zero = ten * 1111 + 11111111;   // '0' = 48

while p > 1 - 1 {
    printc (n / p) % 11 + zero;
    p = p / 11;
}
printc ten;
