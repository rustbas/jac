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

int read_file(const char* filepath, raw_data *rd, int verbose);
int write_to_file(const char* filepath, raw_data *rd);

size_t get_index_from_ft(freq ft[FREQ_TABLE_SIZE], u8 symbol);
int count_freqs(freq ft[FREQ_TABLE_SIZE], raw_data *rd, int verbose);
int freq_comp(const void *a, const void *b);
void print_ft(freq ft[FREQ_TABLE_SIZE], size_t n);

struct Tree {
  char code;
  u8 symbol;
  struct Tree *left_child;
  struct Tree *right_child;
  int isSequence;  
};
typedef struct Tree Tree;



int build_huffman_tree_helper(Tree *tree, freq ft[FREQ_TABLE_SIZE], size_t depth) {
  if (depth == FREQ_TABLE_SIZE-2) {
    Tree *left_sub_tree = malloc(sizeof(Tree));
    Tree *right_sub_tree = malloc(sizeof(Tree));

    left_sub_tree->symbol = ft[depth].symbol;
    left_sub_tree->code = '0';
    left_sub_tree->left_child = NULL;
    left_sub_tree->right_child = NULL;
    left_sub_tree->isSequence = 0;

    right_sub_tree->symbol = ft[depth+1].symbol;
    right_sub_tree->code = '1';
    right_sub_tree->left_child = NULL;
    right_sub_tree->right_child = NULL;
    right_sub_tree->isSequence = 0;

    tree->left_child = left_sub_tree;
    tree->right_child = right_sub_tree;
    tree->isSequence = 0;
    tree->symbol = (u8) 0;
    tree->code = '0';
  } else if (depth == 0) {
    Tree *left_sub_tree = malloc(sizeof(Tree));
    Tree *right_sub_tree = malloc(sizeof(Tree));

    left_sub_tree->symbol = ft[depth].symbol;
    left_sub_tree->code = '0';
    left_sub_tree->left_child = NULL;
    left_sub_tree->right_child = NULL;
    left_sub_tree->isSequence = 0;

    right_sub_tree->symbol = (u8) 0;
    right_sub_tree->code = '1';
    right_sub_tree->isSequence = 1;

    tree->isSequence = 1;
    tree->symbol = (u8)0;
    tree->code = '0';
    tree->left_child = left_sub_tree;
    tree->right_child = right_sub_tree;
    
    build_huffman_tree_helper(right_sub_tree, ft, depth+1);
  }
  else {
    Tree *left_sub_tree = malloc(sizeof(Tree));
    Tree *right_sub_tree = malloc(sizeof(Tree));

    right_sub_tree->code = '1';
    right_sub_tree->symbol = ft[depth].symbol;
    right_sub_tree->left_child = NULL;
    right_sub_tree->right_child = NULL;
    right_sub_tree->isSequence = 0;

    left_sub_tree->symbol = (u8) 0;
    left_sub_tree->code = '0';
    left_sub_tree->isSequence = 1;
    tree->left_child = left_sub_tree;
    tree->right_child = right_sub_tree;
    build_huffman_tree_helper(left_sub_tree, ft, depth+1);
  }
}

int build_huffman_tree(Tree *tree, freq ft[FREQ_TABLE_SIZE]) {
  return build_huffman_tree_helper(tree, ft, 0);
}

void remove_huffman_tree(Tree *tree) {
  if (tree->left_child != NULL)
    remove_huffman_tree(tree->left_child);
  if (tree->right_child != NULL)
    remove_huffman_tree(tree->right_child);
  free(tree);
}

#define STRING_SIZE 256
struct dictionary {
  u8 symbol;
  char code[STRING_SIZE];
};
typedef struct dictionary Dict;

int build_dict_symbol(Tree *tree, Dict *d, size_t depth) {
  if (tree->left_child != NULL)
    if (tree->left_child->isSequence == 0) {
      if (tree->left_child->symbol == d->symbol) {
	d->code[depth] = tree->left_child->code;
	/* d->code[depth+1] = '\0'; */
      } else {
	d->code[depth] = tree->right_child->code;
	build_dict_symbol(tree->right_child, d, depth+1);
      }
    } else {
      if (tree->right_child->symbol == d->symbol) {
	d->code[depth] = tree->right_child->code;
	/* d->code[depth+1] = '\0'; */
      } else {
	d->code[depth] = tree->left_child->code;
	build_dict_symbol(tree->left_child, d, depth+1);
      }
    }
}

char *encode_string(raw_data *rd, Dict huffman_dict[FREQ_TABLE_SIZE]) {
  char *coded_string = (char*) malloc(sizeof(char));
  coded_string[0] = '\0';
  /* coded_string[0] = '\0'; */
  for (size_t i=0; i<rd->size; i++) {
    // TODO: avoid using realloc on each iteration
    char *code = huffman_dict[rd->data[i]].code;
    coded_string = realloc(coded_string, strlen(coded_string) + strlen(code)+1);
    coded_string = strcat(coded_string, code);
  }
  
  size_t to_truncate = ((strlen(coded_string) / 8) + 1) * 8 - strlen(coded_string);
  char *result_string = calloc(to_truncate+1, sizeof(char));

  for (size_t i=0; i<to_truncate; i++)
    result_string[i] = '0';
  
  result_string = realloc(result_string, strlen(result_string) + strlen(coded_string)+1);
  result_string = strcat(result_string, coded_string);
  
  return result_string;
}

u8 chunk_to_num(const char *chunk) {
  u8 result = 0;
  u8 base, pow;
  size_t len = strlen(chunk); // We can do that, cause chunk allocated on stack
  for (size_t i=0; i<len; i++) {
    base = chunk[i] - '0';
    result += base * (1 << (len - i - 1));
  }
  return result;
}

int main(size_t argc, char **argv) {
  int verbose = 0;
  char input_file[BUFFER_SIZE];
  assert(argc == 3);
  
  for (size_t i=0; i<argc; i++) {
    if (strcmp(argv[i], "-i") == 0)
      strcpy(input_file, argv[++i]);
  }
  
  freq ft[FREQ_TABLE_SIZE] = {0};
  for (size_t i=0; i<FREQ_TABLE_SIZE; i++)
    ft[i].symbol = (u8)i;
  
  raw_data rd = {0};

  read_file(input_file, &rd, verbose);
  count_freqs(ft, &rd, verbose);

  size_t n = 15;
  /* printf("First %d elements before sort: \n", n); */
  /* print_ft(ft, n); */

  qsort(ft, FREQ_TABLE_SIZE, sizeof(freq), freq_comp);
  /* printf("First %d elements after sort: \n", n); */
  /* print_ft(ft, FREQ_TABLE_SIZE); */
  Tree *tree = malloc(sizeof(Tree));
  build_huffman_tree(tree, ft);

  Dict huffman_dict[FREQ_TABLE_SIZE];
  for (size_t i=0; i<FREQ_TABLE_SIZE; i++)
    huffman_dict[i].symbol = (u8)i;

  /* size_t shift = 0x47; */

  for (size_t shift=0; shift < FREQ_TABLE_SIZE; shift++) {
    build_dict_symbol(tree, &huffman_dict[shift], 0);
  }

  size_t shifts[] = {0x0A, 0x61, 0x62, 0x63};
  size_t shift;
  for (size_t i=0; i < sizeof(shifts) / sizeof(shifts[0]); i++) {
    shift = shifts[i];
    if (verbose)
      printf("    0x%02X: %s\n",
	     huffman_dict[shift].symbol,
	     huffman_dict[shift].code);   
  }

  char *result_string = encode_string(&rd, huffman_dict);

  size_t chunk_size = 8;
  size_t chunks_num = strlen(result_string) / chunk_size;
  assert(strlen(result_string) % chunk_size == 0);

  u8 *compressed_data = malloc(sizeof(u8)*chunks_num);
  
  for (size_t i=0; i<chunks_num; i++) {
    size_t idx = i*8;
    char msg[16] = {0};
    memcpy(msg, result_string+idx, sizeof(result_string[0])*8);
    compressed_data[i] = chunk_to_num(msg);
    printf("  %02X - %s\n", compressed_data[i], msg);
  }
  
  printf("%d - %s\n", strlen(result_string), result_string);

  const char *result = "result.jacz";
  FILE *output_file = fopen(result, "wb");
  assert(output_file != NULL);

  fwrite(compressed_data, chunks_num, 1, output_file);
  
  fclose(output_file);
  free(rd.data);
  free(result_string);
  free(compressed_data);
  remove_huffman_tree(tree);
  return 0;
}
