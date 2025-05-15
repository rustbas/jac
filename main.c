#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNIMPLEMENTED							\
  do {									\
    fprintf(stderr, "%s:%d: UNIMPLEMENTED\n", __FILE__, __LINE__);	\
    exit(1);								\
  } while(0);

#define BUFFER_SIZE 1024

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

struct Tree {
  char code;
  u8 symbol;
  struct Tree *left_child;
  struct Tree *right_child;
  int isSequence;  
};
typedef struct Tree Tree;

int read_file(const char* filepath, raw_data *rd, int verbose);
int write_to_file(const char* filepath, raw_data *rd);

size_t get_index_from_ft(freq ft[FREQ_TABLE_SIZE], u8 symbol);
int count_freqs(freq ft[FREQ_TABLE_SIZE], raw_data *rd, int verbose);
void print_ft(freq ft[FREQ_TABLE_SIZE], size_t n);

int build_huffman_tree_helper(Tree *tree, freq ft[FREQ_TABLE_SIZE], size_t depth);

int build_huffman_tree(Tree *tree, freq ft[FREQ_TABLE_SIZE]);

void remove_huffman_tree(Tree *tree);

#define STRING_SIZE 256
struct dictionary {
  u8 symbol;
  char code[STRING_SIZE];
};
typedef struct dictionary Dict;

int build_dict_symbol(Tree *tree, Dict *d, size_t depth);
void build_huffman_dict(Dict huffman_dict[FREQ_TABLE_SIZE], Tree *tree);
char *encode_string(raw_data *rd, Dict huffman_dict[FREQ_TABLE_SIZE], size_t *to_truncate);
#define CHUNK_SIZE 8

u8 chunk_to_num(const char chunk[CHUNK_SIZE]);
void num_to_chunk(char buffer[CHUNK_SIZE], u8 data);
ssize_t get_index_from_dict(Dict dict[FREQ_TABLE_SIZE], const char *code);
u8 *convert_string_to_array(size_t chunks_num, const char *result_string);
void write_data_to_file(const char *filepath,
			Dict huffman_dict[FREQ_TABLE_SIZE],
			size_t to_truncate,
			size_t chunks_num,
			u8 *compressed_data);
u8 *read_from_file(const char *filepath,
		   Dict *dict,
		   size_t *to_truncate,
		   size_t *chunks_num);

char *convert_data_to_string(size_t chunks_num,
			     u8 *compressed_data);
void decode_to_file(const char *filepath,
		    const char *string_for_decoding,
		    Dict *dict);

void compress_file(const char *input_file,
		   const char *archive_file,
		   int verbose) {
  // Building frequency table
  freq ft[FREQ_TABLE_SIZE] = {0};
  raw_data rd = {0};

  read_file(input_file, &rd, verbose);
  count_freqs(ft, &rd, verbose);

  // Building huffman tree
  Tree *tree = malloc(sizeof(Tree));
  build_huffman_tree(tree, ft);

  // Building huffman dictionary
  Dict huffman_dict[FREQ_TABLE_SIZE];
  build_huffman_dict(huffman_dict, tree);

  // Encoding data to binary string
  size_t to_truncate = 0;
  char *result_string = encode_string(&rd, huffman_dict, &to_truncate);

  size_t chunks_num = strlen(result_string) / CHUNK_SIZE;
  assert(strlen(result_string) % CHUNK_SIZE == 0);

  // Converting binary string to byte-array
  u8 *compressed_data = convert_string_to_array(chunks_num, result_string);
 
  // Writing data to file
  // TODO: use argv

  write_data_to_file(archive_file,
      huffman_dict,
      to_truncate,
      chunks_num,
      compressed_data);

  free(rd.data);
  free(result_string);
  free(compressed_data);
  remove_huffman_tree(tree);
};

void extract_file(const char *archive_file,
		  const char *result) {
  // Reading data from file
  Dict huffman_dict_readed[FREQ_TABLE_SIZE];
  size_t to_truncate_readed = 0;
  size_t chunks_num_readed = 0;
  u8 *compressed_data_readed;
  
  compressed_data_readed = read_from_file(archive_file,
					  huffman_dict_readed,
					  &to_truncate_readed,
					  &chunks_num_readed);

  // Convering raw data from file to binary string
  char *data_string = convert_data_to_string(chunks_num_readed,
					     compressed_data_readed);

  // Writing binary string to file
  char *string_for_decoding = data_string+to_truncate_readed;
  size_t begin = 0, len = 1;

  decode_to_file(result,
		 string_for_decoding,
		 huffman_dict_readed);

  free(data_string);
};

int main(size_t argc, char **argv) {
  // CLI args parsing
  int verbose = 0;
  char input_file[BUFFER_SIZE];
  assert(argc == 3);
  
  for (size_t i=0; i<argc; i++) {
    if (strcmp(argv[i], "-i") == 0)
      strcpy(input_file, argv[++i]);
  }

  const char *archive_name = "result.jacz";
  compress_file(input_file, archive_name, 0);

  const char *extracted_file = "test.txt";
  extract_file(archive_name, extracted_file);

  return 0;
}
