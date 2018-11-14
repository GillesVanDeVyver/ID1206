#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {

    int pid = fork();

    if(pid == 0) {
        int child = getpid();
        printf("Child: Session %d\n", getsid(child));
    } else {
        int parent = getpid();
        printf("Parent: Session %d\n", getsid(parent));
    }

    return 0;
}

