let s = 1 - 1;
let i = 1;
while i < 111111 {
    s = s + i;
    i = i + 1;
}
print s;
print i;
