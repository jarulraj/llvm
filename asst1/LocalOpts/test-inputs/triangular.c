#include <stdio.h>

unsigned triangularNumber(unsigned n);

const char *suffix[] = {" ","st","nd","rd"};

int main(int argc, char *argv[]) {
	int n = 0,m;
	unsigned t;

	if ((argc < 2) ||
	(sscanf(argv[1],"%d",&n) != 1) ||
	(n <= 0)) {
		printf("\nusage %s <n>\n",argv[0]);
		printf(" where <n> is a positive integer\n");
		return -1;
	}
    t = (int)(n * (((float)n + 1.0) / 2.0));
    t *= 3;
    if (t == triangularNumber(n)) {
		m = n % 10;
		printf("\nThe %d%s triangular number is %d\n",
		n,
		((m > 0) && (m < 4)) ? suffix[m] : "th",
		t);
	} else {
		printf("\nerror\n");
	}
	return 0;
}

unsigned triangularNumber(unsigned n) {
	if (n == 1) return 1;
	return n + triangularNumber(n-1);
}
