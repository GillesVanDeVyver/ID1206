#include <stdio.h>

#include "green.h"

void *test(void *arg) {
    int i = *(int*) arg;
    int loop = 10;
    while (loop > 0) {
      for (int i = 0; i < 100000000; i++);
      printf("thread %d: %d\n", i, loop);
      loop--;
      //green_yield();
    }
}

int main() {
  green_t g0, g1, g2;
  int a0 = 0;
  int a1 = 1;
  int a2 = 2;

  //First test
  green_create(&g0, test, &a0);
  green_create(&g1, test, &a1);
  green_create(&g2, test, &a2);
  green_join(&g0);
  green_join(&g1);
  green_join(&g2);

  printf("Done with the test.\n");
  return 0;
}
