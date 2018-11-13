#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	
	int counter = 0;

	while(counter !=10){
		counter++;
		printf("Hello %s (%p) %d\n", argv[1], &counter, counter);
		sleep(1);
	}
	return 0;
}
