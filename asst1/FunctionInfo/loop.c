int g;
int g_incr (int c)
{
    g += c;
    return g;
}
int loop (int a, int b, int c)
{
    int i;
    int ret = 0;
    for (i = a; i < b; i++) {
        g_incr (c);
    }
    return ret + g;
}

// Only 1 block
void z()
{
}

int y(int a)
{
    int s = loop(1, 100, 3);

    if(s > 0)
        y(a);

    return s;
}


// Var Arg
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

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
