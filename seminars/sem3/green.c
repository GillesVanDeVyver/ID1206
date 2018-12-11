#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>

#include "green.h"

#define FALSE 0
#define TRUE 1

#define STACK_SIZE 4096

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, FALSE};

static green_t *running = &main_green;

void green_thread();


//The ready queue
static green_t *readyQueue;

static void init() __attribute__((constructor));

void init() {
  getcontext(&main_cntx);
}

/*
  Adds a thread to the ready queue
*/
void enqueue (green_t *new) {
  green_t *curr_queue = readyQueue;

  if (readyQueue != NULL) {
    //Finds the end of the ready queue
    while (curr_queue->next != NULL) {
      curr_queue = curr_queue->next;
    }
    //Adds new to the end of the queue
    curr_queue->next = new;
  } else {
    readyQueue = new;
  }
}

/*
  Takes out the next thread from the ready queue
  Returns NULL if the queue is empty
*/
struct green_t *dequeue () {
  green_t *nextThread = NULL;

  if (readyQueue != NULL) {
    nextThread = readyQueue;
    readyQueue = readyQueue->next;
  }
  nextThread->next = NULL;
  return nextThread;
}

int green_create (green_t *new, void *(*fun)(void*), void *arg) {
  ucontext_t *cntx = (ucontext_t *) malloc(sizeof(ucontext_t));
  getcontext(cntx);

  void *stack = malloc(STACK_SIZE);

  cntx->uc_stack.ss_sp = stack;
  cntx->uc_stack.ss_size = STACK_SIZE;

  makecontext(cntx, green_thread, 0);
  new->context = cntx;
  new->fun = fun;
  new->arg = arg;
  new->next = NULL;
  new->join = NULL;
  new->zombie = FALSE;

  /*
    Add "new" to the ready queue
  */
  enqueue(new);

  return 0;
}

void green_thread() {
  green_t *this = running;

  (*this->fun)(this->arg);

  //Place waiting (joining) thread in ready queue
  if (this->join != NULL) {
    enqueue(this->join);
  }

  //Free alocated memory structures
  free(this->context->uc_stack.ss_sp);
  free(this->context);

  //We're a zombie
  this->zombie = TRUE;

  //Find the next thread to run
  green_t *next = dequeue();
  if (next == NULL) {
    next = &main_green;
  }

  running = next;
  setcontext(next->context);
}

int green_yield () {
  green_t * susp = running;

  //Add susp to ready queue
  enqueue(susp);

  //Select the next thread for execution
  struct green_t *next = dequeue();

  running = next;
  swapcontext(susp->context, next->context);
  return 0;
}

int green_join (green_t *thread) {

  if (thread->zombie) {
    return 0;
  }

  green_t *susp = running;

  //Add to waiting threads
  thread->join = susp;

  //Select the next thread for execution
  green_t *next = dequeue();
  if (next == NULL) {
    next = &main_green;
  }

  running = next;
  swapcontext(susp->context, next->context);

  return 0;
}
