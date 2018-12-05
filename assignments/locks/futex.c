#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>

volatile int count = 0;

volatile int global = 0;

typedef struct args {
  int inc;
  int id;
  volatile int *mutex;
} args;

int try(volatile int *mutex) {
  return __sync_val_compare_and_swap(mutex, 0, 1);
}

int futex_wait(volatile int *futexp) {
  return syscall(SYS_futex, futexp, FUTEX_WAIT, 1, NULL, NULL, 0);
}

int futex_wake(volatile int *futexp) {
  syscall(SYS_futex, futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
}

/*
    The thread will continouosly try to grab the lock.
*/
int lock(volatile int *lock) {
  int susp = 0;
  while(try(lock) != 0){
    susp++;
    futex_wait(lock);
  }
  return susp;
}

/*
    Sets the mutex to 0 in order to unlock the lock
*/
void unlock(volatile int *lock) {
  *lock = 0;
  futex_wake(lock);
}

void *increment(void *arg) {
  int inc = ((args*)arg)->inc;
  int id = ((args*)arg)->id;
  volatile int *mutex = ((args*)arg)->mutex;
  int spin = 0;

  for (int i = 0; i < inc; i++) {
    spin += lock(mutex);
    count++;
    unlock(mutex);
  }
  return (void *)(unsigned long int) spin;
}

/*
    Will start two threads and wait for them to finish
    before writing out the value of the counter
*/
int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage Peterson <inc>\n");
    exit(0);
  }

  int inc = atoi(argv[1]);

  pthread_t one_p, two_p;
  args one_args, two_args;

  one_args.inc = inc;
  two_args.inc = inc;

  one_args.id = 0;
  two_args.id = 1;

  one_args.mutex = &global;
  two_args.mutex = &global;

  pthread_create(&one_p, NULL, increment, &one_args);
  pthread_create(&two_p, NULL, increment, &two_args);
  int retVal1, retVal2;
  pthread_join(one_p, (void*)&retVal1);
  pthread_join(two_p, (void*)&retVal2);

  printf("Start %d\n", retVal1);
  printf("Start %d\n", retVal2);
  printf("Result is: %d\n", count);
  return 0;
}
