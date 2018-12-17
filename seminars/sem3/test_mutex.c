#include <stdio.h>

#include "green.h"

volatile int shared_counter = 0;
green_mutex_t mutex;
int zero, one, two;
void *mutex_test(void *arg) {
  int id = *(int *) arg;
  int incr = 1000000;

  for(int i = 0; i < 100000; i++);
  for (int i = 0; i < incr; i++) {
    green_mutex_lock(&mutex);
    if (id == 0) {
      zero++;
    } else if (id == 1) {
      one++;
    } else {
      two++;
    }
    shared_counter++;
    green_mutex_unlock(&mutex);
  }
}

int main() {
  green_t g0, g1, g2;
  int a0 = 0;
  int a1 = 1;
  int a2 = 2;

  green_mutex_init(&mutex);
  green_create(&g0, mutex_test, &a0);
  green_create(&g1, mutex_test, &a1);
  green_create(&g2, mutex_test, &a2);

  green_join(&g0);
  green_join(&g1);
  green_join(&g2);
  printf("Zero: %d | One: %d | Two: %d\n", zero, one, two);
  printf("Shared counter result: %d\n", shared_counter);
  printf("Mutex test is done.\n");

  return 0;
}
