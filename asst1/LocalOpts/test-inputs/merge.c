int compute ()
{
  int result = 0;
  int a = 2;
  int b = 3;
  int c = 4 + a + b;
  int d = 5;

  result += a;
  result += b;
  
  result = b / 32;
  result += 0;
  result = 0 + result;
  result -= 0;
 
  result += a * 2;
  result = d * 8;
  result -= b / 2;
 
  result *= 0;
  result *= 1;

  result -= b / 4;
  result *= (b/b);
  result += (b-b);
  result /= result;
  result -= result;
  //result = result && 0;
  
  result = a * 8;
  result *= c;
  result /= 2;
  return result;
} 
