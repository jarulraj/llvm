
int main() {
    int n = 5, m;
    unsigned int t;
    int x, y;
    int argc = 3;     

    if ((argc < 2) || (n <= 0)) {
        return -1;
    }

    for (n = 0 ; n < 10 ; n++)
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
                n = n % 3;
                t = n * 2;
            } 
            else {
                t = 5 * 6;    
            }
        }

        t = n * 2;
        x = m / 5;
        y = 5 * 6;    
    }

    return argc;
}

