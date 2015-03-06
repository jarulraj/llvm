int main ()
{
  int result;
  int n = 1;

  while (n++ < 100) {
	  result = 0;
	  result += 2;
	  result += 3;
	  result *= 4;
	  result /= 2;
  }

  return result;
}
