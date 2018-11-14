#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(){

	char *heap = malloc(20);
	*heap = 0x61;
	printf("Heap pointing to: 0x%x\n", *heap);


	free(heap);

	char *foo = malloc(20);
	*foo = 0x62;
	printf("Foo pointing to: 0x%x\n", *foo);

	*heap = 0x63;
	printf("Or is it pointing to: 0x%x\n", *foo);

	return 0;
}