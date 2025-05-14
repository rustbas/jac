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

#define STRING_SIZE 512
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

int main(size_t argc, char **argv) {
  int verbose = 1;
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
	     /* huffman_dict[shift].symbol, */
	     huffman_dict[shift].code);   
  }

  /* for (size_t i=0; i<rd.size; i++) */
  /*   printf(" 0x%02X", rd.data[i]); */
  /* printf("\n"); */
  /* for (size_t i=0; i<rd.size; i++) */
  /*   printf("%5s", huffman_dict[rd.data[i]].code); */
  /* printf("\n"); */

  // TODO: Concat using realloc
  char *result_string = (char*) malloc(sizeof(char));
  result_string[0] = '\0';
  /* result_string[0] = '\0'; */
  for (size_t i=0; i<rd.size; i++) {
    // TODO: avoid using realloc on each iteration
    char *code = huffman_dict[rd.data[i]].code;
    result_string = realloc(result_string, strlen(result_string) + strlen(code)+1);
    result_string = strcat(result_string, code);
  }
  
  size_t to_truncate = ((strlen(result_string) / 8) + 1) * 8 - strlen(result_string);
  char *header_string = calloc(to_truncate+1, sizeof(char));

  for (size_t i=0; i<to_truncate; i++)
    header_string[i] = '0';
  header_string = realloc(header_string, strlen(header_string) + strlen(result_string)+1);
  header_string = strcat(header_string, result_string);
      
  for (size_t i=0; i<strlen(header_string)/8; i++) {
    size_t idx = i*8;
    char *msg[16] = {0};
    memcpy(msg, header_string+idx, sizeof(char)*8);
    printf("%s\n", msg);
  }

  
  printf("%d - %s\n", strlen(header_string), header_string);
  printf("%d\n", to_truncate);
  
  free(rd.data);
  free(result_string);
  free(header_string);
  remove_huffman_tree(tree);
  return 0;
}



