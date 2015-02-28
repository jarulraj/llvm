int fun(int x)
{
	int y = 0;
	int w = 0;
	for (int i=0; i<10;i++) {
		y = w+2;
		w = y+2;
	}
	return x;
}
