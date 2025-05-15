#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#define FREQ_TABLE_SIZE 256

typedef unsigned char u8;

typedef struct {
  u8 *data;
  size_t size;
} raw_data;

struct Tree {
  char code;
  u8 symbol;
  struct Tree *left_child;
  struct Tree *right_child;
  int isSequence;  
};
typedef struct Tree Tree;

typedef struct {
  u8 symbol;
  size_t count;
} freq;

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
      } else {
	d->code[depth] = tree->right_child->code;
	build_dict_symbol(tree->right_child, d, depth+1);
      }
    } else {
      if (tree->right_child->symbol == d->symbol) {
	d->code[depth] = tree->right_child->code;
      } else {
	d->code[depth] = tree->left_child->code;
	build_dict_symbol(tree->left_child, d, depth+1);
      }
    }
}

void build_huffman_dict(Dict huffman_dict[FREQ_TABLE_SIZE], Tree *tree) {
  for (size_t i=0; i<FREQ_TABLE_SIZE; i++)
    huffman_dict[i].symbol = (u8)i;

  for (size_t shift=0; shift < FREQ_TABLE_SIZE; shift++) {
    build_dict_symbol(tree, &huffman_dict[shift], 0);
  }
}

char *encode_string(raw_data *rd, Dict huffman_dict[FREQ_TABLE_SIZE], size_t *to_truncate) {
  /* char *coded_string = (char*) malloc(sizeof(char)); */
  /* coded_string[0] = '\0'; */
  /* for (size_t i=0; i<rd->size; i++) { */
  /*   char *code = huffman_dict[rd->data[i]].code; */
  /*   coded_string = realloc(coded_string, strlen(coded_string) + strlen(code)+1); */
  /*   coded_string = strcat(coded_string, code); */
  /* } */
  size_t coded_string_len = 1;
  for (size_t i=0; i<rd->size; i++) {
    coded_string_len += strlen(huffman_dict[rd->data[i]].code);
  }

  char *coded_string = (char*) calloc(sizeof(char), coded_string_len);
  for (size_t i=0; i<rd->size; i++)
    coded_string = strcat(coded_string, huffman_dict[rd->data[i]].code);
  
  *to_truncate = ((strlen(coded_string) / 8) + 1) * 8 - strlen(coded_string);
  char *result_string = calloc(*to_truncate+1, sizeof(char));

  for (size_t i=0; i<*to_truncate; i++)
    result_string[i] = '0';
  
  result_string = realloc(result_string, strlen(result_string) + strlen(coded_string)+1);
  result_string = strcat(result_string, coded_string);
  
  return result_string;
}

#define CHUNK_SIZE 8

u8 chunk_to_num(const char chunk[CHUNK_SIZE]) {
  u8 result = 0;
  u8 base, pow;
  size_t len = strlen(chunk); // We can do that, cause chunk allocated on stack
  for (size_t i=0; i<len; i++) {
    base = chunk[i] - '0';
    result += base * (1 << (len - i - 1));
  }
  return result;
}

void num_to_chunk(char buffer[CHUNK_SIZE], u8 data) {
  for (size_t i=0; i < CHUNK_SIZE; i++)
    buffer[CHUNK_SIZE - i - 1] = '0' + (data >> i) % 2;
}

ssize_t get_index_from_dict(Dict dict[FREQ_TABLE_SIZE], const char *code) {
  for (ssize_t i=0; i<FREQ_TABLE_SIZE; i++)
    if (strcmp(dict[i].code, code) == 0)
      return i;
  return -1;
}

u8 *convert_string_to_array(size_t chunks_num, const char *result_string) {
  u8 *result = malloc(sizeof(u8)*chunks_num);
  
  for (size_t i=0; i<chunks_num; i++) {
    size_t idx = i*8;
    char msg[16] = {0};
    memcpy(msg, result_string+idx, sizeof(result_string[0])*8);
    result[i] = chunk_to_num(msg);
  }

  return result;
}

void write_data_to_file(const char *filepath,
	 Dict huffman_dict[FREQ_TABLE_SIZE],
	 size_t to_truncate,
	 size_t chunks_num,
	 u8 *compressed_data) {
  // TODO: use raw_data's functions
  FILE *output_file = fopen(filepath, "wb");
  assert(output_file != NULL);
  
  fwrite(huffman_dict, sizeof(Dict)*FREQ_TABLE_SIZE, 1, output_file);
  fwrite(&to_truncate, sizeof(to_truncate), 1, output_file);
  fwrite(&chunks_num, sizeof(chunks_num), 1, output_file);
  fwrite(compressed_data, chunks_num, 1, output_file);

  fclose(output_file);
}

u8 *read_from_file(const char *filepath,
		    Dict *dict,
		    size_t *to_truncate,
		    size_t *chunks_num) {
  FILE *fd = fopen(filepath, "rb");
  assert(fd != NULL);
  
  fread(dict, sizeof(Dict)*FREQ_TABLE_SIZE, 1, fd);
  fread(to_truncate, sizeof(size_t), 1, fd);
  fread(chunks_num, sizeof(size_t), 1, fd);
  size_t cn = *chunks_num;
  u8 *compressed_data = (u8*) malloc(sizeof(u8)*cn);
  fread(compressed_data, sizeof(u8)*cn, 1, fd);

  fclose(fd);
  return compressed_data;
}

char *convert_data_to_string(size_t chunks_num,
			     u8 *compressed_data) {
  char *data_string = calloc(sizeof(char),(chunks_num * 8)+1);
  
  for (size_t i=0; i<chunks_num; i++) {
    char buffer[CHUNK_SIZE] = {0};
    num_to_chunk(buffer, compressed_data[i]);

    char *data_string_cursor = data_string + i*8;
    memcpy(data_string_cursor, buffer, sizeof(char)*CHUNK_SIZE);
  }

  return data_string;
}

void decode_to_file(const char *filepath,
		    const char *string_for_decoding,
		    Dict *dict) {
  FILE *test_file = fopen("test.txt", "wb");
  size_t begin = 0, len = 1;
  while (begin < strlen(string_for_decoding)) {
    char buffer[STRING_SIZE+1] = {0};
    memcpy(buffer, string_for_decoding+begin, sizeof(char)*len);
    ssize_t idx = get_index_from_dict(dict, buffer);
    if (idx == -1) {
      len++;
      continue;
    } else {
      u8 symbol = dict[idx].symbol;
      fwrite(&symbol, sizeof(u8), 1, test_file);
      begin += len;
      len = 1;
    }
  };
  fclose(test_file);
}
