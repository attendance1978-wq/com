// Compile this to C and build a native binary:
//   build/com build examples/04_c_interop.com --emit c -o out.c
//   cc -O2 -o out out.c && ./out
let a = 1 - 1;
let b = 1;
let n = 111111111111111;
let i = 1 - 1;
while i < n {
    print a;
    let t = a + b;
    a = b;
    b = t;
    i = i + 1;
}
