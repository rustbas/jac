#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;

typedef struct {
  u8 *data;
  size_t size;
} raw_data;

int read_file(const char* filepath, raw_data *rd, int verbose) {

  // TODO: verbose, assertions and errors checks
  
  FILE *file = fopen(filepath, "rb");

  fseek(file, 0, SEEK_END); // Jump to the EOF
  rd->size = ftell(file);   // Get offset of file
  rewind(file);             // Return to begin of file

  rd->data = (u8*) malloc(rd->size * sizeof(u8));
  fread(rd->data, rd->size, 1, file);

  if (verbose) {
    printf("[INFO] Readed %zu bytes from %s\n", rd->size, filepath);
    printf("[INFO] First 10 bytes:\n    ");
    for (size_t i=0; i<10; i++)
      printf("%2X ", rd->data[i]);
    printf("\n");
  }
  
  fclose(file);
  return 0;
}

int write_to_file(const char* filepath, raw_data *rd) {
  FILE *file = fopen(filepath, "wb");
  fwrite(rd->data, rd->size, 1, file);
  fclose(file);
  return 0;
}
