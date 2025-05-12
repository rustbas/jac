#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

typedef unsigned char u8;

typedef struct {
  u8 *data;
  size_t size;
} raw_data;

typedef struct {
  u8 symbol;
  size_t count;
} freq;

#define FREQ_TABLE_SIZE 256

size_t get_index_from_ft(freq ft[FREQ_TABLE_SIZE], u8 symbol) {
  for (size_t i=0; i<FREQ_TABLE_SIZE; i++)
    if (ft[i].symbol == symbol)
      return i;
  fprintf(stderr, "[ERR] Unreachable: %2X\n", symbol);
  exit(69);
}

int count_freqs(freq ft[FREQ_TABLE_SIZE], raw_data *rd, int verbose) {
  if (verbose)
    printf("[INFO] Counting frequencies...\n");
  for (size_t i = 0; i<rd->size; i++) {
    /* size_t idx = get_index_from_ft(ft, rd->data[i]);  */
    size_t idx = (size_t) rd->data[i];
    ft[idx].count++;
  }
    
  if (verbose == 2) {
    for (size_t i=0; i<32; i++) {
      for (size_t j=0; j<8; j++) {
	printf("%02X: %9zu ", i*8+j, ft[i*8+j].count);
      }
      printf("\n");
    }
  }
}

int freq_comp(const void *a, const void *b) {
  freq *a_new = (freq*) a;
  freq *b_new = (freq*) b;
  return -(a_new->count - b_new->count);
}

void print_ft(freq ft[FREQ_TABLE_SIZE], size_t n) {
  n = (n <= FREQ_TABLE_SIZE) ? n : FREQ_TABLE_SIZE;
  for (size_t i=0; i<n; i++)
    if (isprint(ft[i].symbol))
      printf("    FT['%c'] = %zu\n", ft[i].symbol, ft[i].count);
    else
      printf("    FT[%X] = %zu\n", ft[i].symbol, ft[i].count);
}
