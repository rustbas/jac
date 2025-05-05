#include <stdio.h>
#include <stdlib.h>

#define UNIMPLEMENTED							\
do {									\
	fprintf(stderr, "%s:%d: UNIMPLEMENTED\n", __FILE__, __LINE__);	\
	exit(1);							\
} while(0);

#define FILEPATH "data/data.fa"
#define FILEPATH2 "data/data2.fa"

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
/* typedef freq freq_table[FREQ_TABLE_SIZE]; */

int read_file(const char* filepath, raw_data *rd, int verbose);
int write_to_file(const char* filepath, raw_data *rd);

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

int main(size_t argc, char *argv) {
  int verbose = 2;

  freq ft[FREQ_TABLE_SIZE] = {0};
  for (size_t i=0; i<FREQ_TABLE_SIZE; i++)
    ft[i].symbol = (u8)i;
  
  raw_data rd = {0};

  read_file(FILEPATH, &rd, verbose);

  count_freqs(ft, &rd, verbose);
  
  free(rd.data);
  return 0;
}



