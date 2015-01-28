int loop (int a, int b, int c);

// TEST 1 : Only 1 block
void z()
{
}

// TEST 2 : Recursion
int y(int a)
{
    int s = loop(1, 100, 3);

    if(s > 0)
        y(a);

    return s;
}


// TEST 3 : Var Arg
#include <stdarg.h>     

int FindMax (int n, ...)
{
    int i,val,largest;
    va_list vl;
    va_start(vl,n);
    largest=va_arg(vl,int);
    for (i=1;i<n;i++)
    {
        val=va_arg(vl,int);
        largest=(largest>val)?largest:val;
    }
    va_end(vl);
    return largest;
}
