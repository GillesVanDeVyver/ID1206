#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>

#include "green.h"

typedef struct p_arg {
  int id;
  int sum;
  pthread_mutex_t *mutex;
} p_arg;

typedef struct g_arg {
  int id;
  int sum;
  green_mutex_t *mutex;
} g_arg;

static int shared_counter = 0;
void *p_test (void *arg) {
  int id = ((p_arg*) arg)->id;
  int sum = ((p_arg*) arg)->sum;
  pthread_mutex_t *mutex = ((p_arg*) arg)->mutex;
  printf("Pthread id: %d | Sum: %d\n", id, sum);

  for (int i = 0; i < sum; i++) {
    pthread_mutex_lock(mutex);
    shared_counter++;
    pthread_mutex_unlock(mutex);
  }
}

static int shared_counter_g = 0;
void *g_test (void *arg) {
  int id = ((p_arg*) arg)->id;
  int sum = ((p_arg*) arg)->sum;
  green_mutex_t *g_mutex = ((g_arg*) arg)->mutex;
  printf("Gthread id: %d | Sum: %d\n", id, sum);

  for (int i = 0; i < sum; i++) {
    green_mutex_lock(g_mutex);
    shared_counter_g++;
    green_mutex_unlock(g_mutex);
  }
}

int main () {
  clock_t c_start, c_stop;
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);
  pthread_t t1;
  p_arg ptarg1 = {0,5000000, &mutex};
  pthread_t t2;
  p_arg ptarg2 = {1, 5000000, &mutex};
  pthread_t t3;
  p_arg ptarg3 = {2, 5000000, &mutex};

  c_start = clock();
  pthread_create(&t1, NULL, p_test, (void *)&ptarg1);
  pthread_create(&t2, NULL, p_test, (void *)&ptarg2);
  pthread_create(&t3, NULL, p_test, (void *)&ptarg3);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  pthread_join(t3, NULL);
  c_stop = clock();

  double total = ((double) c_stop - c_start) / CLOCKS_PER_SEC;
  total = total*10000;
  printf("%.2f %d\n", total, shared_counter);

  /*********************************************
  *               GREEN THREADS                *
  **********************************************/
  clock_t cg_start, cg_stop;
  green_mutex_t *g_mutex;
  green_mutex_init(g_mutex);
  green_t g0, g1, g2;
  g_arg gtarg1 = {0, 5000000, g_mutex};
  g_arg gtarg2 = {1, 5000000, g_mutex};
  g_arg gtarg3 = {2, 5000000, g_mutex};

  cg_start = clock();
  green_create(&g0, g_test, &gtarg1);
  green_create(&g1, g_test, &gtarg2);
  green_create(&g2, g_test, &gtarg3);

  green_join(&g0);
  green_join(&g1);
  green_join(&g2);
  cg_stop = clock();

  double total_g = ((double) c_stop - c_start) / CLOCKS_PER_SEC;
  total_g = total*10000;
  printf("%.2f %d\n", total_g, shared_counter_g);

  return 0;
}
