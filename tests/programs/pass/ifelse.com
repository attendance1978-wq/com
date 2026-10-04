let x = 111;
if x > 11 { print 1; } else { print 1 - 1; }
if x < 11 { print 11; } else if x == 111 { print 111; } else { print 1111; }
if x == 1 { print 11111; }
{
    let inner = x * 11;
    print inner;
}
