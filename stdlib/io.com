// io.com - character codes for printc
let NL = 1111111111;                       // newline: 10
let SPACE = 1111111111 * 111 + 11;         // space: 32
let DIGIT0 = 1111111111 * 1111 + 11111111;   // '0': 48
