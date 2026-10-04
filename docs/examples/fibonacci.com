// First 10 Fibonacci numbers.
let a = 1 - 1;
let b = 1;
let i = 1 - 1;
while i < 1111111111 {
    print a;
    let t = a + b;
    a = b;
    b = t;
    i = i + 1;
}
