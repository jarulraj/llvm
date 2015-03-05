int compute ()
{
  int result;
  while (1) {
	  result = 0;
	  int a = 2;
	  int b = 3;
	  int c = 4 + a + b;
	  
	  result += a;
	  result += b;
	  result *= c;
	  result /= 2;
	  if(result)
	  	break;
  }
  return result;
}
