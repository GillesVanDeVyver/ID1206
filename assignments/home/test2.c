#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int done = 0;

void handler(int sig, siginfo_t *siginfo, void *context){
  printf("Signal %d was caught\n", sig);

  printf("Your UID is %d\n", siginfo->si_uid);
  printf("Your PID is %d\n", siginfo->si_pid);

  done = 1;
}

int main(){
  struct sigaction sa;

  int pid = getpid();

  printf("OK, let's go - kill me (%d) and I'll tell you who you are.\n", pid);

  /*We're using the more elaborated sigaction handler*/
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = handler;

  sigemptyset(&sa.sa_mask);

  if(sigaction(SIGINT, &sa, NULL) != 0){
    return(1);
  }

  while(!done){}

  printf("Told you so!\n");
  return(0);
}
