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

volatile int shared_counter = 0;
green_mutex_t mutex;

int zero = 0;
int one = 0;
int two = 0;

void *mutex_test(void *arg) {
  int id = *(int *) arg;
  int incr = 1000000;

  printf("Thread %d adds %d to shared counter.\n", id, incr);
  for(int i = 0; i < 100000; i++);
  for (int i = 0; i < incr; i++) {
    green_mutex_lock(&mutex);
    shared_counter++;
    if (id == 0)
      zero++;
    else if (id == 1)
      one++;
    else
      two++;
    green_mutex_unlock(&mutex);
  }
}

int flag = 0;
green_cond_t cond;

void *test_cond(void *arg) {
    int id = *(int *) arg;
    int loop = 4;
    while (loop > 0) {
        if (flag == id) {
            printf("Thread %d: %d\n", id, loop);
            loop--;
            flag = (id + 1) % 3;
            green_cond_signal(&cond);
        } else {
            green_cond_wait(&cond);
        }
    }
}

int main() {
  green_t g0, g1, g2;
  int a0 = 0;
  int a1 = 1;
  int a2 = 2;

  //First test
  //green_create(&g0, test, &a0);
  //green_create(&g1, test, &a1);
  //green_create(&g2, test, &a2);

  //Condition test
  //green_cond_init(&cond);
  /*green_create(&g0, test_cond, &a0);
  green_create(&g1, test_cond, &a1);
  green_create(&g2, test_cond, &a2);*/

  //Mutex lock
  green_mutex_init(&mutex);
  green_create(&g0, mutex_test, &a0);
  green_create(&g1, mutex_test, &a1);
  green_create(&g2, mutex_test, &a2);

  green_join(&g0);
  green_join(&g1);
  green_join(&g2);
  printf("Zero: %d | One: %d | Two: %d\n", zero, one, two);
  printf("Shared counter result: %d\n", shared_counter);
  printf("Done with the test.\n");
  return 0;
}
