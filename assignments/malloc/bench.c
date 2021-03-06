#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "rand.h"

#define ROUNDS 10
#define LOOP 100000
#define BUFFER 100

int main(){
  void *buffer[BUFFER];
  for(int i = 0; i < BUFFER; i++){
    buffer[i] = NULL;
  }

  void *init = sbrk(0);
  void *current;

  printf("The initial top of the heap is %p.\n", init);

  for(int i = 0; i < ROUNDS; i++){
    for(int j = 0; j < LOOP; j++){
      int index = rand() % BUFFER;
      if(buffer[index] != NULL){
        free(buffer[index]);
      }
      //Requesting more memory
      size_t size = (size_t) request();
      int *memory;
      memory = malloc(size);

      if(memory == NULL){
        fprintf(stderr, "Memory allocation failed\n");
        return(1);
      }
      /*Writing to the memory so we know it exists*/
      *memory = 123;
      free(memory);
    }
    current = sbrk(0);
    int allocated = (int)((current-init) / 1024);
    printf("%d\n", i);
    printf("The current top of the heap is %p. \n", current);
    printf("Increased by %d Kbyte\n", allocated);
  }
  return 0;
}
