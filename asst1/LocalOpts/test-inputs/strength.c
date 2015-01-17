int compute (int a, int b)
{
  int result = 0;
  
  result += a * 2;
  result += a * 3;
  result += a * 8;
  result -= b / 2;
  result -= b / 4;
  result -= b / 8;
  return result;
}
