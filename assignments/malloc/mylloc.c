#include <stdlib.h>
#include <unistd.h>

void *malloc(size_t size){
  if(size == 0){
    return NULL;
  }
  void *memory = sbrk(size);
  //if OS fails to increase heap segment. -1 will be
  //returned. Malloc should return NULL
  if(memory == (void *)-1){
    return NULL;
  } else {
    return memory;
  }
}


void free (void *memory){
  return;
}
