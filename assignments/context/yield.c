#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>

#define MAX 10

static int running;

static ucontext_t cntx_one;
static ucontext_t cntx_two;
static ucontext_t cntx_main;
static ucontext_t cntx_done;
static int done1;
static int done2;

void done() {
  int done = 0;

  while(!done) {
    if (running == 1) {
      printf(" - Process one terminating -\n");
      done1 = 1;
      if (!done2) {
        running = 2;
        swapcontext(&cntx_done, &cntx_two);
      } else {
        done = 1;
      }
    } else {
      printf(" - Process two terminating -\n");
      done2 = 1;
      if (!done1) {
        running = 1;
        swapcontext(&cntx_done, &cntx_one);
      } else {
        done = 1;
      }
    }
  }
  printf(" - Done terminating -\n");
}

/*
    Switching between context one and two, depening on which
    context that is executing
*/
void yield() {
  printf(" - yield -\n");
  // if context 1 is running
  if (running == 1) {
    running = 2;
    swapcontext(&cntx_one, &cntx_two);
  } else {
    // if context 2 is running
    running = 1;
    swapcontext(&cntx_two, &cntx_one);
  }
}

/*
    Serves the purpose of making the stack grow
*/
void push(int p, int i) {
  if (i < MAX) {
    printf("%d%*s PUSH\n", p, i, " ");
    push(p, i+1);
    printf("%d%*s POP\n", p, i, " ");
  } else {
    printf("%d%*s TOP\n", p, i, " ");
    yield();
  }
}

int main() {
  char stack1[8*1024];
  char stack2[8*1024];
  char stack_done[8*1024];

  /*
    The first context
  */
  getcontext(&cntx_one);
  cntx_one.uc_link = &cntx_done;
  cntx_one.uc_stack.ss_sp = stack1;
  cntx_one.uc_stack.ss_size = sizeof(stack1);
  makecontext(&cntx_one, (void (*) (void)) push, 2, 1, 1);

  /*
    The second context
  */
  getcontext(&cntx_two);
  cntx_two.uc_link = &cntx_done;
  cntx_two.uc_stack.ss_sp = stack2;
  cntx_two.uc_stack.ss_size = sizeof(stack2);
  makecontext(&cntx_two, (void (*) (void)) push, 2, 2, 1);

  getcontext(&cntx_done);
  cntx_done.uc_link = &cntx_main;
  cntx_done.uc_stack.ss_sp = stack_done;
  cntx_done.uc_stack.ss_size = sizeof(stack_done);
  makecontext(&cntx_done, (void (*) (void)) done, 0);

  running = 1;

  printf(" - LET'S GO! - \n");
  swapcontext(&cntx_main, &cntx_one);
  printf(" - THAT'S ALL FOLKS\n");

  return 0;
}
