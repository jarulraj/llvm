int test () {
	int j;
	int y;
	int z;
	int res = 0;
	for (int i = 0; i < 20; i++) {
		j = 5;
		int k = i+j;
		res += k;
		for (int x = 0; x < 10; x++) {
			y = 10;
			y = 7;
			z = 5;
			int t = y+z;
			res += t;
		}
	}
	return res;
}
