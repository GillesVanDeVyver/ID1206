#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#ifdef __linux__
#include <string.h>
#elif __APPLE__
#include <strings.h>
#endif
#include <math.h>

#define MIN 5
#define LEVELS 8
#define PAGE 4096

struct head *find(int index);
void insert(struct head*);
void test_headers(struct head*);
void print_mem();

struct head *flists[LEVELS] = {NULL};

enum flag {Free = 1337, Taken = 666}; // To make test_headers print unknown

struct head {
    enum flag status;
    short int level;
    struct head *next;
    struct head *prev;
};

struct head *new() {
    struct head *new = (struct head*) mmap(NULL, PAGE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (new == MAP_FAILED) {
        return NULL;
    }
    assert(((long int)new & 0xfff) == 0); // last 12 bits should be zero
    new->status = Free;
    new->level = LEVELS - 1;
    return new;
}

struct head *buddy(struct head *block) {
    int index = block->level;
    long int mask = 0x1 << (MIN + index);
    return (struct head*)((long int) block ^ mask); // ^ == XOR
}

struct head *split(struct head *block) {
    int index = block->level - 1;
    int mask = 0x1 << (index + MIN);
    struct head *split = (struct head*) ((long int) block | mask);
    split->level = index;
    split->status = Free;
    block->level = index;
    return split;
}

struct head *primary(struct head *block) {
    int index = block->level;
    long int mask = 0xffffffffffffffff << (MIN + index + 1);
    struct head *prim = (struct head*) ((long int) block & mask);
    return prim;
}

/* Gives the address to the next address to hide access to the header */
void *hide(struct head* block) {
    return (void *) (block + 1 );
}

/* Backs the address one step in order to retrive the header from a block */
struct head *magic(struct head* memory) {
    return ((struct head*) memory - 1);
}

/* Finds what block-level is required to allocate the requested amount of memory */
int level(int req) {
    int total = req + sizeof(struct head);

    int lvl = 0;
    int size = 1 << MIN;
    while (total > size) {
        size <<= 1;
        lvl += 1;
    }
    return lvl;
}
/* allocate a block */
void *balloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    int index = level(size);
    struct head *taken = find(index);
    return hide(taken);
}

/* free a memory block */
void bfree(void *memory) {
    if (memory != NULL) {
        struct head *block = magic(memory);
        block->status = Free;
        insert(block);
    }
    return;
}

void split_up(int level, int goal) {
    if(level <= goal){
      return;
    }
    //Unlink the block from the list, and split it up
    struct head *block = flists[level];
    flists[level] = flists[level]->next;
    if(flists[level] != NULL){
      flists[level]->prev = NULL;
    }
    struct head *supl = split(block);
    //Link these into the lower level
    block->prev = NULL;
    block->next = flists[block->level];
    if(flists[block->level] != NULL){
      flists[block->level]->prev = block;
    }
    flists[block->level] = block;

    supl->prev = NULL;
    supl->next = flists[supl->level];
    if(flists[supl->level] != NULL){
      flists[supl->level]->prev = supl;
    }
    flists[block->level] = supl;
    split_up(level - 1, goal);
}

struct head *find(int index) {
    printf("Trying to allocate mem of level %d\n", index);
    if (flists[index] == NULL){
      int level_w_mem = index + 1;
      while (flists[level_w_mem] == NULL && level_w_mem < LEVELS ) {
        level_w_mem++;
      }
      if(level_w_mem == LEVELS){
        printf("Not enough memory, need more!\n");
        exit(1);
      }
      printf("Highest memory index: %d\n", level_w_mem);
      split_up(level_w_mem, index);
    }
    struct head *found = flists[index];
    found->status = Taken;
    flists[index] = flists[index]->next;
    if(flists[index] != NULL){
      flists[index]->prev = NULL;
    }
    found->next = NULL;
    return found;
}
/*
    Inserts a memory block in the free list.
    Checks if their buddy is free and in that case merge the two.
*/
void insert(struct head *block) {
  int level = block->level;
  /*
    Have to check if the current free block is the largest block 4KB
  */
  if(level == LEVELS - 1){
    block->prev = NULL;
    block->next = flists[level];
    if (flists[level] != NULL){
      flists[level]->prev = block;
    }
    flists[level] = block;
  } else {
    struct head *bud = buddy(block);
    struct head *prim = primary(block);
    /*
        When you don't have a free buddy
        Add just the requested block to the free list
        Without any consideration of their buddy
        Their buddy is on another level
    */
    if (bud->status == Taken || bud->level != block->level){
      block->prev = NULL;
      block->next = flists[level];
      if(flists[level] != NULL){
        flists[level]->prev = block;
      }
      flists[level] = block;
    } else {
      /*
        When you have the buddy and the primary.
        Add to the free list but as merged
      */
        if(prim->prev == NULL /*&& bud->level == block->level*/){
          flists[level] = prim->next;
          if(flists[level] != NULL){
            prim->next->prev = NULL;
          }
        }
        prim->level = level + 1;
        insert(prim);
    }
  }
}

void test_headers(struct head *mem) {
    if (mem == NULL) {
        printf("ERROR: NULL HEADER\n");
        return;
    }
    char status[20];
    if (mem->status == Free) {
        strcpy(status, "Free");
    } else if (mem->status == Taken) {
        strcpy(status, "Taken");
    } else {
        strcpy(status, "Unknown");
    }
    printf("\nBlock: %p\n", mem);
    printf("mem->status: %s\n", status);
    printf("mem->level:  %d (%d KB)\n", mem->level, (int) pow(2, mem->level+MIN));
    printf("mem->next:   %p\n", mem->next);
    printf("mem->prev:   %p\n", mem->prev);
}

void print_mem() {
    int i;
    struct head *curr;
    for (i = 0 ; i < LEVELS; i++) {
        printf("\nLevel %d: \n", i);
        if (flists[i] != NULL) {
            curr = flists[i];
            while (curr != NULL) {
                test_headers(curr);
                curr = curr->next;
            }
        }
    }
}

void test() {
    // Test the headers
    printf("\n=== Create ===\n");
    struct head *mem = new();
    test_headers(mem);
    printf("\n=== Obfuscate ===\n");
    struct head *obfsk_mem = (struct head *) hide(mem);
    test_headers(obfsk_mem);
    printf("\n=== Recover ===\n");
    struct head *recovered_mem = (struct head *) magic(obfsk_mem);
    test_headers(recovered_mem);

    // Divide it
    printf("\n=== Splitting ===\n");
    struct head *halved = split(mem);
    test_headers(mem);
    test_headers(halved);

    printf("\n=== Splitting AGAIN ===\n");
    struct head *quarter = split(halved);
    printf("quarter:\n");
    test_headers(quarter);
    printf("halve:\n");
    test_headers(halved);

    printf("\n=== Primary ===\n");
    struct head *prim = primary(quarter);
    test_headers(prim);
}

void test2() {
  insert(new());
  char *myMem = balloc(20*sizeof(int));
  char *myMem2 = balloc(40*sizeof(int));
  printf("==== HEADER TESTS ====\n\n");
  printf("THIS IS THE BLOCK THAT I GOT TO MY MEMORY:");
  test_headers(magic((struct head*)myMem));
  test_headers(magic((struct head*)myMem2));
  print_mem();
  printf("\n");
  printf("\n==== FREEING MEMORY ======");
  bfree(myMem2);
  print_mem();
  bfree(myMem);
  print_mem();

}
