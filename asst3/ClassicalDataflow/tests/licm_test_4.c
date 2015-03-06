int test () {
	int j;
	int x;
	int y;
	int z;
	int res = 0;
	for (int i = 0; i < 20; i++) {
		j = 5;
		int k = i+j;
		res += k;
		if (res) {
			y = 6;			
		}
		else {
			z = 7;
		}
		for (int t = 0; t < 10; ++t) {
			j = 10;
			x = 5;
			int d = t+x;
			res += d;
		}
	}
	x = y+2;
	z = j+4;
	return res;
}
