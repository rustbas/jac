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
} binary_data;

int read_file(const char* filepath, binary_data *bd) {

  // TODO: verbose, assertions and errors checks
  
  FILE *file = fopen(filepath, "rb");

  fseek(file, 0, SEEK_END); // Jump to the EOF
  bd->size = ftell(file);   // Get offset of file
  rewind(file);             // Return to begin of file

  bd->data = (u8*) malloc(bd->size * sizeof(u8));
  fread(bd->data, bd->size, 1, file);

  printf("[INFO] Readed %zu bytes from %s\n", bd->size, filepath);
  printf("[INFO] First 10 bytes:\n    ");
  for (size_t i=0; i<10; i++)
    printf("%2X ", bd->data[i]);
  printf("\n");
  
  fclose(file);
  return 0;
}

int close_file(const char* filepath, binary_data *bd) {

  FILE *file = fopen(filepath, "wb");
  fwrite(bd->data, bd->size, 1, file);

  fclose(file);
  
  return 0;
}

int main(size_t argc, char *argv) {
  printf("hello, world!\n");

  binary_data bd;

  read_file(FILEPATH, &bd);

  close_file(FILEPATH2, &bd);
  free(bd.data);
  return 0;
}
