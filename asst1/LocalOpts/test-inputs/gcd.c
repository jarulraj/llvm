int gcd(int a, int b) {
  if (b == 0)
    return a;

  a *= 1;
  a *= 2;
  a /= 2;

  return gcd(b, a % b);
}
