#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {

    int pid = fork();

    if(pid == 0) {
        int child = getpid();
        printf("Child: parent %d, group %d\n", getppid(), getpgid(child));
        sleep(4);
        printf("Child: parent %d, group %d\n", getppid(), getpgid(child));
        sleep(4);
        printf("Child: parent %d, group %d\n", getppid(), getpgid(child));
    } else {
        int parent = getpid();
        printf("Parent: parent %d, group %d\n", getppid(), getpgid(parent));
        sleep(2);
        int zero = 0;
        int i = 3 / zero;
    }

    return 0;
}

