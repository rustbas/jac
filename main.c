#include <stdio.h>
#include <stdlib.h>

#define UNIMPLEMENTED							\
do {									\
	fprintf(stderr, "%s:%d: UNIMPLEMENTED\n", __FILE__, __LINE__);	\
	exit(1);							\
} while(0);

#define FILEPATH "data/data.fa"
#define FILEPATH2 "data/data2.fa"

size_t verbose = 0;

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
typedef freq freq_table[FREQ_TABLE_SIZE];

int read_file(const char* filepath, raw_data *rd, int verbose);
int write_to_file(const char* filepath, raw_data *rd);

size_t get_index_from_ft(freq_table *ft, u8 symbol) {
  for (size_t i=0; i<FREQ_TABLE_SIZE; i++)
    if (ft[i]->symbol == symbol)
      return i;
  exit(6969);
}

int count_freqs(unsigned long *ft, raw_data *rd) {
  if (verbose)
    printf("[INFO] Counting frequencies...\n");
  for (size_t i = 0; i<rd->size; i++)
    ft[rd->data[i]]++;

  if (verbose == 2) {
    for (size_t i=0; i<32; i++) {
      for (size_t j=0; j<8; j++) {
	printf("%02X: %9zu ", i*8+j, ft[i*8+j]);
      }
      printf("\n");
    }
  }
}

int main(size_t argc, char *argv) {
  verbose = 1;

  unsigned long freq_table[256] = {0};
  raw_data rd;

  read_file(FILEPATH, &rd, verbose);

  count_freqs(freq_table, &rd);
  
  write_to_file(FILEPATH2, &rd);
  free(rd.data);
  return 0;
}



