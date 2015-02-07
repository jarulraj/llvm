int sum (int a , int b)
{
 int i;
 int res = 1;

 for ( i = a; i < b; i++)
 {
     res *= i;
 }

 return res;
}

