
int foo(int argc, char *argv[]) {
    int n, m;
    unsigned int t;
    int x, y;

    if ((argc < 2) || (n <= 0)) {
        return -1;
    }

    n = argc;
    m = n + 1;
    t = n * 5;
    t = m / 2;

    if (t == n) {
        m = n % 3;
        n = n % 3;
        m = n % 3;

        t = n * 2;
        m = m / 5;
    } 
    else {
        m /= 5;
        m = n % 3;
        t = 5 * 6;    
    }

    t = n * 2;
    x = m / 5;
    n = n % 3;
    y = 5 * 6;    
    n = m * 2;

    return argc;
}

