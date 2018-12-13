#include <stdio.h>

#include "green.h"

static green_cond_t cond;
static green_mutex_t cond_mutex;
static int flag = 0;

void *mutex_cond_test(void *arg) {
  int id = *(int *) arg;
  int loop = 10;
  while (loop > 0) {
    green_mutex_lock(&cond_mutex);
    while (1) {
      if (flag == id) {
        printf("Thread %d: %d\n", id, loop);
        flag = (id + 1) % 3;
        green_cond_signal(&cond);
        green_mutex_unlock(&cond_mutex);
        break;
      } else {
        green_cond_wait(&cond, &cond_mutex);
      }
    }
    loop--;
  }
}

int main() {
  green_t g0, g1, g2;
  int a0 = 0;
  int a1 = 1;
  int a2 = 2;

  green_mutex_init(&cond_mutex);
  green_cond_init(&cond);
  green_create(&g0, mutex_cond_test, &a0);
  green_create(&g1, mutex_cond_test, &a1);
  green_create(&g2, mutex_cond_test, &a2);
  green_join(&g0);
  green_join(&g1);
  green_join(&g2);
  printf("Done with condition and mutex test.\n");

  return 0;
}
