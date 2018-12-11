#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>

typedef struct args {
    int inc;
    int id;
    volatile int *mutex;
} args;

volatile int count = 0;
volatile int global = 0;

int futex_wait(volatile int *futexp) {
    return syscall(SYS_futex, futexp, FUTEX_WAIT, 1, NULL, NULL, 0);
}

void futex_wake(volatile int *futexp) {
    syscall(SYS_futex, futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
}

int try(volatile int *mutex) {
    return __sync_val_compare_and_swap(mutex, 0, 1);
}

int lock(volatile int *mutex) {
    int spin = 0;
    while (try(mutex) != 0) {
        spin++;
        futex_wait(mutex);
    }
    return spin;
}

void unlock(volatile int *mutex) {
    *mutex = 0;
    futex_wake(mutex);
}

void *increment(void *arg) {
    int inc = ((args*)arg)->inc;
    int id = ((args*)arg)->id;
    unsigned long int spun = 0;
    volatile int *mutex = ((args*)arg)->mutex;

    for (int i = 0; i < inc; i++) {
        spun += lock(mutex);
        count++;
        unlock(mutex);
    }
    return (void *)spun;
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("usage peterson <inc>\n");
        exit(0);
    }

    int inc = atoi(argv[1]);

    pthread_t one_p, two_p;
    args one_args, two_args;

    one_args.mutex = &global;
    two_args.mutex = &global;
    one_args.inc = inc;
    two_args.inc = inc;
    one_args.id = 0;
    two_args.id = 1;

    pthread_create(&one_p, NULL, increment, &one_args);
    pthread_create(&two_p, NULL, increment, &two_args);

    int retval1, retval2;
    pthread_join(one_p, (void*) &retval1);
    pthread_join(two_p, (void*) &retval2);

    printf("first returned: %d\n", retval1);
    printf("second returned: %d\n", retval2);
    printf("result is %d\n", count);

    return 0;
}
