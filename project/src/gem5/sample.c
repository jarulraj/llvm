#include <stdio.h>
/*
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <smmintrin.h>

#define PATTERN_ROW 0
#define PATTERN_COL 7
*/

/*
static long gather(long *table, int index, int pattern) {
  char *p = (char*)&table[index] + pattern;
  long result;
  //asm("prefetch %1\n" : "=a"(result) : "m"(*p) : );
  return result;
}

static __m128i gather16(long *table, int index, int pattern) {
  char *p = (char*)&table[index] + pattern;
  __m128i result;
  //asm("prefetcht0 %1\n" : "=Yz"(result) : "m"(*p) : );
  return result;
}

static __attribute__((aligned(512))) long table[] = {

  // Cache line 0
  0x0000000000000000,
  0x0101010101010101,
  0x0202020202020202,
  0x0303030303030303,
  0x0404040404040404,
  0x0505050505050505,
  0x0606060606060606,
  0x0707070707070707,

  // Cache line 1
  0x1010101010101010,
  0x1111111111111111,
  0x1212121212121212,
  0x1313131313131313,
  0x1414141414141414,
  0x1515151515151515,
  0x1616161616161616,
  0x1717171717171717,

  // Cache line 2
  0x2020202020202020,
  0x2121212121212121,
  0x2222222222222222,
  0x2323232323232323,
  0x2424242424242424,
  0x2525252525252525,
  0x2626262626262626,
  0x2727272727272727,

  // Cache line 3
  0x3030303030303030,
  0x3131313131313131,
  0x3232323232323232,
  0x3333333333333333,
  0x3434343434343434,
  0x3535353535353535,
  0x3636363636363636,
  0x3737373737373737,

  // Cache line 4
  0x4040404040404040,
  0x4141414141414141,
  0x4242424242424242,
  0x4343434343434343,
  0x4444444444444444,
  0x4545454545454545,
  0x4646464646464646,
  0x4747474747474747,

  // Cache line 5
  0x5050505050505050,
  0x5151515151515151,
  0x5252525252525252,
  0x5353535353535353,
  0x5454545454545454,
  0x5555555555555555,
  0x5656565656565656,
  0x5757575757575757,

  // Cache line 6
  0x6060606060606060,
  0x6161616161616161,
  0x6262626262626262,
  0x6363636363636363,
  0x6464646464646464,
  0x6565656565656565,
  0x6666666666666666,
  0x6767676767676767,

  // Cache line 7
  0x7070707070707070,
  0x7171717171717171,
  0x7272727272727272,
  0x7373737373737373,
  0x7474747474747474,
  0x7575757575757575,
  0x7676767676767676,
  0x7777777777777777
};
*/

int main(int argc, char **argv) {
  
    /*
  union {
    __m128i packed;
    struct {
      long lo;
      long hi;
    };
  } result;
  int i;

  for (i = 0; i < 16; i++) {
    printf("table[%2d] by tuple: %016lx\n", i, gather(table, i, PATTERN_ROW));
  }

  for (i = 0; i < 16; i++) {
    printf("table[%2d] by field: %016lx\n", i, gather(table, i, PATTERN_COL));
  }

  for (i = 0; i < 16; i += 2) {
    result.packed = gather16(table, i, PATTERN_ROW);
    printf("table[%2d:%2d] by tuple: %016lx%016lx\n", i+1, i, result.hi,
        result.lo);
  }

  for (i = 0; i < 16; i += 2) {
    result.packed = gather16(table, i, PATTERN_COL);
    printf("table[%2d:%2d] by field: %016lx%016lx\n", i+1, i, result.hi,
        result.lo);
  }

  printf("Changing table\n");
  for (i = 0; i < 64; i++) {
    table[i] |= 0xff;
  }

  for (i = 0; i < 16; i += 2) {
    result.packed = gather16(table, i, PATTERN_ROW);
    printf("table[%2d:%2d] by tuple: %016lx%016lx\n", i+1, i, result.hi,
        result.lo);
  }

  for (i = 0; i < 16; i += 2) {
    result.packed = gather16(table, i, PATTERN_COL);
    printf("table[%2d:%2d] by field: %016lx%016lx\n", i+1, i, result.hi,
        result.lo);
  }
*/

  return 0;
}

