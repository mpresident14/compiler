#include <stdio.h>

void printInt(long n) {
  printf("%ld\n", n);
}

extern void runprez() asm("runprez");

int main() {
  runprez();
}
