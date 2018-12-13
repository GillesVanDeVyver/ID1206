#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include "green.h"

#define FALSE 0
#define TRUE 1

#define PERIOD 100
#define STACK_SIZE 4096

void green_thread(void);
green_t *dequeue(green_t**);
void enqueue(green_t**, green_t*);
void timer_handler(int);

static sigset_t block;

static green_mutex_t *mutex;

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, FALSE};

static green_t *running = &main_green;

static green_t *readyQueue = NULL;

static void init() __attribute__((constructor));

void init() {
  getcontext(&main_cntx);

  sigemptyset(&block);
  sigaddset(&block, SIGVTALRM);

  struct sigaction act = {0};
  struct timeval interval;
  struct itimerval period;

  act.sa_handler = timer_handler;
  assert(sigaction(SIGVTALRM, &act, NULL) == 0);

  interval.tv_sec = 0;
  interval.tv_usec = PERIOD;
  period.it_interval = interval;
  period.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &period, NULL);
}

void timer_handler (int sig) {
  sigprocmask(SIG_BLOCK, &block, NULL);
  green_t *susp = running;

  //Add the running to the ready queue
  enqueue(&readyQueue, susp);

  //Find the next thread for execution
  green_t *next = dequeue(&readyQueue);
  running = next;
  sigprocmask(SIG_UNBLOCK, &block, NULL);
  swapcontext(susp->context, next->context);
}

/*
  Returns the next in the ready queue.
  Returns the main_green thread if the queue is empty
*/
green_t *dequeue(green_t **list) {
  if (*list == NULL) {
    printf("Emil this happened at line 71...\n");
    return NULL;
  } else {
    green_t *thread = *list;
    *list = (*list)->next;
    thread->next = NULL;
    return thread;
  }
}

/* adds a thread to the ready queue. */
void enqueue(green_t **list, green_t *new) {
  if (*list == NULL) {
    *list = new;
  } else {
    green_t *curr = *list;
    while (curr->next != NULL) {
      curr = curr->next;
    }
    curr->next = new;
  }
}

/* creates a thread with a function and parameters */
int green_create(green_t *new, void *(*fun)(void*), void *arg) {
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

  // add new to the ready queue
  enqueue(&readyQueue, new);

  return 0;
}

/*
    Starts the execution of a function (of a thread)
    and then terminate the thread
*/
void green_thread() {
  green_t *this = running;
  (*this->fun)(this->arg);

  // Place waiting (joining) thread in ready queue
  if (this->join != NULL) {
    green_t *thread = this->join;
    this->join = this->join->next;
    enqueue(&readyQueue, thread);
  }

  // Free allocated memory structures
  free(this->context->uc_stack.ss_sp);
  free(this->context);

  // we're a zombie
  this->zombie = TRUE;

  // find the next thread to run
  green_t *next = dequeue(&readyQueue);
  running = next;
  setcontext(next->context);
}

/* yields a thread from execution */
int green_yield() {
  green_t *susp = running;
  enqueue(&readyQueue, susp);

  green_t *next = dequeue(&readyQueue);
  running = next;
  swapcontext(susp->context, next->context);
  // IMPORTANT POINT IN CODE, EXECUTION WILL RESUME HERE FOR SUSP!!!!
  return 0;
}

/* waits a the specified thread to terminate before executing */
int green_join(green_t *thread) {
  if (thread->zombie)
    return 0;

  // add susp to the ready queue
  green_t *susp = running;
  if (this->join != NULL) {
    enqueue(&readyQueue, this->join);
  }

  // select the next thread for execution
  green_t *next = dequeue(&readyQueue);
  running = next;
  swapcontext(susp->context, next->context);

  return 0;
}

/************************************************
 *                CONDITIONALS                  *
 * **********************************************/

void green_cond_init(green_cond_t *cond) {
    cond->num_susp = 0;
    cond->waiting = NULL;
}

/* Suspends the process on the condition variable */
void green_cond_wait(green_cond_t *cond) {
  //Block timer interrupt
  sigprocmask(SIG_BLOCK, &block, NULL);
  // The currently running one will be suspended
  green_t *susp = running;

  // Add the suspended thread to the list
  enqueue(&cond->waiting, susp);

  // Find next to run and run it.
  green_t *next = dequeue(&readyQueue);

  running = next;
  cond->num_susp++;
  swapcontext(susp->context, next->context);
}

/* Signals the next waiting on the variable */
void green_cond_signal(green_cond_t *cond) {
  if (cond->num_susp == 0) {
    assert(cond->waiting == NULL);
    return;
  }
  assert(cond->waiting != NULL && cond->num_susp > 0);
  green_t *old = cond->waiting;
  cond->waiting = old->next;

  old->next = NULL;
  enqueue(&readyQueue, old);
  cond->num_susp--;
}

/***************************
*        MUTEX LOCKS       *
***************************/

int green_mutex_init(green_mutex_t *mutex) {
  mutex->taken = FALSE;
  mutex->susp = NULL;
}

int green_mutex_lock (green_mutex_t *mutex) {
  //Block timer interrupt
  sigprocmask(SIG_BLOCK, &block, NULL);

  green_t *susp = running;

  while (mutex->taken) {
    //Suspend the running thread
    enqueue(&mutex->susp, susp);
    //Find the next thread
    green_t *next = dequeue(&readyQueue);
    running = next;

    swapcontext(susp->context, next->context);
  }
  //Take the lock
  mutex->taken = TRUE;
  //Unblock the timer interrupt
  sigprocmask(SIG_UNBLOCK, &block, NULL);
  return 0;
}

int green_mutex_unlock (green_mutex_t *mutex) {
  //Block timer interrupt
  sigprocmask(SIG_BLOCK, &block, NULL);
  //Move suspended threads to ready queue
  if (mutex->susp != NULL) {
    enqueue(&readyQueue, mutex->susp);
  }

  //Release the lock
  mutex->taken = FALSE;
  mutex->susp = NULL;

  //Unblock the timer interrupt
  sigprocmask(SIG_UNBLOCK, &block, NULL);
}
