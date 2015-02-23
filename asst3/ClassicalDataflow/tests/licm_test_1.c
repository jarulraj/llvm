
int foo(int argc, char *argv[]) {
    int n, m;
    unsigned int t;
    int x, y;

    if ((argc < 2) || (n <= 0)) {
        return -1;
    }

    for (n = 0 ; n < 100 ; n++)
    {
        m = n + 1;
        t = n * 5;
        t = m / 2;

        for(m = 1 ; m < 10 ; m++)
        {
            while(t > 0){
                t--;
            }

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
        }

        t = n * 2;
        x = m / 5;
        n = n % 3;
        y = 5 * 6;    
        n = m * 2;
    }

    return argc;
}

