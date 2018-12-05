#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

volatile int count = 0;

volatile int request[2] = {0, 0};
volatile int turn = 0;

typedef struct args {
  int inc;
  int id;
} args;

/*
    To take the lock, the thread will set its own flag to 1
    and then wait until either the other thread is not interested
    or it is its turn.
*/
void lock(int id) {
  request[id] = 1;
  int other = 1 - id;
  turn = other;
  while(request[other] == 1 && turn == other) {}; //SPIN
}

/*
    To release a lock, the thread restes its request flag
*/
void unlock(int id) {
  request[id] = 0;
}

void *increment(void *arg) {
  int inc = ((args*)arg)->inc;
  int id = ((args*)arg)->id;

  for (int i = 0; i < inc; i++) {
    lock(id);
    count++;
    unlock(id);
  }
  return (void *)(unsigned long int) id;
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
