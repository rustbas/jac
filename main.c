#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNIMPLEMENTED							\
do {									\
	fprintf(stderr, "%s:%d: UNIMPLEMENTED\n", __FILE__, __LINE__);	\
	exit(1);							\
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
  if (depth == FREQ_TABLE_SIZE-1) {
    Tree *left_sub_tree = malloc(sizeof(Tree));
    Tree *right_sub_tree = malloc(sizeof(Tree));

    left_sub_tree->symbol = ft[depth].symbol;
    left_sub_tree->code = '0';
    left_sub_tree->left_child = NULL;
    left_sub_tree->right_child = NULL;
    left_sub_tree->isSequence = 0;

    right_sub_tree->symbol = ft[depth].symbol;
    right_sub_tree->code = '1';
    right_sub_tree->left_child = NULL;
    right_sub_tree->right_child = NULL;
    right_sub_tree->isSequence = 0;

    tree->left_child = left_sub_tree;
    tree->right_child = right_sub_tree;
    tree->isSequence = 0;
    tree->symbol = (u8) 0;
    tree->code = (char) 0;
  } else if (depth == 0) {
    Tree *left_sub_tree = malloc(sizeof(Tree));
    Tree *right_sub_tree = malloc(sizeof(Tree));

    left_sub_tree->symbol = ft[depth].symbol;
    left_sub_tree->code = '0';
    left_sub_tree->left_child = NULL;
    left_sub_tree->right_child = NULL;
    left_sub_tree->isSequence = 0;

    right_sub_tree->symbol = (u8) 0;
    right_sub_tree->code = (char) 0;
    right_sub_tree->isSequence = 1;

    tree->isSequence = 1;
    tree->left_child = (struct Tree*) left_sub_tree;
    tree->right_child = (struct Tree*) right_sub_tree;
    
    build_huffman_tree(right_sub_tree, ft, depth+1);
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
    left_sub_tree->code = (char) 0;
    left_sub_tree->isSequence = 1;
    tree->left_child = (struct Tree*) left_sub_tree;
    tree->right_child = (struct Tree*) right_sub_tree;
    build_huffman_tree(left_sub_tree, ft, depth+1);
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
  /* print_ft(ft, n); */
  Tree *tree = malloc(sizeof(Tree));
  build_huffman_tree(tree, ft, 0);

  printf("%c\n", tree->code);
  
  free(rd.data);
  return 0;
}



