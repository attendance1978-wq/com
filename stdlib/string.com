// string.com - ASCII codes of common characters, for printc
let CH_A = 1111111111 * 111111 + 11111;        // 'A': 65
let CH_a = 1111111111 * 111111111 + 1111111;        // 'a': 97
let CH_BANG = 1111111111 * 111 + 111;           // '!': 33
