#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#define HIGH 20
#define FREQ 80
#define PAGES 100
#define SAMPLES 20

typedef struct pte {
  int id;
  int present;
  struct pte *next;
  struct pte *prev;
} pte;

void init(int *sequence, int refs, int pages){
  int high = (int)(pages*((float)HIGH/100));
  if(high < 2) high = 2;

  /* dummy value */
  int prev = pages;

  for(int i = 0; i<refs; i++) {
    if(rand()%100 < FREQ) {
      int rnd;
      do {
	/* we want to prevent the same page being picked again */
	rnd = rand()%high;
      } while (rnd == prev);
      prev = rnd;
      sequence[i] = rnd;
    } else {
      int rnd;
      do {
	rnd = high + rand()%(pages - high);
      } while (rnd == prev);
      prev = rnd;
      sequence[i] = rnd;
    }
  }

  /*srand(time(NULL));

  int high = (int)(pages*(float)HIGH/100);

  for (int i = 0; i < refs; i++){
    if(rand()%100 < FREQ){
      The frequently case
      sequence[i] = rand()%high;
    } else {
      sequence[i] = high + rand()%(pages-high);
    }
  }*/
}

void clear_page_table(pte *page_table, int pages){
  for(int i = 0; i < pages; i++) {
    page_table[i].id = i;
    page_table[i].present = 0;
    page_table[i].next = NULL;
    page_table[i].prev = NULL;
  }
}

int simulate (int *seq, pte *table, int refs, int frames, int pages){
  int hits = 0;
  int allocated = 0;

  pte *first = NULL;
  pte *last = NULL;


  for(int i = 0; i < refs; i++) {
    int next = seq[i];
    pte *entry = &table[next];

    if(entry->present == 1) {
      hits++;
      if(entry->next != NULL) {
	/* unlink the entry and place it last */

	if(first == entry) {
	  first = entry->next;
	} else {
	  entry->prev->next = entry->next;
	}
	entry->next->prev = entry->prev;

	entry->prev = last;
	entry->next = NULL;

	last->next = entry;
	last = entry;
      }
    } else {
      if(allocated < frames) {
	allocated++;
	entry->present = 1;
	entry->prev = last;
	entry->next = NULL;

	if(last != NULL) {
	  last->next = entry;
	}
	if(first == NULL) {
	  first = entry;
	}
	last = entry;

      } else {
	pte *evict;

	assert(first != NULL);

	evict = first;
	first = evict->next;

	evict->present = 0;

	entry->present = 1;
	entry->prev = last;
	entry->next = NULL;

	if(last != entry) {
	  last->next = entry;
	}

	if(first == NULL) {
	  first = entry;
	}
	last = entry;
      }
    }
  }

  return hits;
}

int main(int argc, char *argv[]){
  /* Could be command line arguments */
  int refs = 1000000;
  int pages = 100;
  pte table[PAGES];

  int *sequence = (int*)malloc(refs*sizeof(int));

  init(sequence, refs, pages);

  printf("# This is a benchmark of random replacement\n");
  printf("# %d page reference\n", refs);
  printf("# %d pages \n", pages);
  printf("#\n#\n#frames\tratio\n");

  /* Frames is the sixe of the memory in frames */
  int frames;

  int incr = pages/SAMPLES;

  for(frames = incr; frames <= pages; frames += incr){
    /* Clear page tables entries */
    clear_page_table(table, pages);

    int hits = simulate(sequence, table, refs, frames, pages);

    float ratio = (float)hits/refs;

    printf("%d\t%.2f\n", frames, ratio);
  }

  return 0;
}
