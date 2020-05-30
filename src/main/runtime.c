#include <stdio.h>

void printInt(long n) {
  printf("%ld\n", n);
}

extern void setup() asm("__setup");

int main() {
  setup();
}
