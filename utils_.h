#ifndef UTILS__H
#define UTILS__H

#define ENOMEM 12
#define SUCCESS 0
#define INVALID_VALUE 1
#define FULL_STACK 2
#define HASH_TABLE_SIZE 200
#define STACK_SIZE 100
#define NAME_SIZE 100
#define STDIN "stdin"
#define STDOUT "stdout"
#define INVALID_NR_PARAMS 3
#define O_READ "rb"
#define O_WRITE "wb"
#define INVALID_FILE 4
#define LINE_SIZE 256
#define LINE_WORDS 100
#define WORD_SIZE 50
#define MAPPING_SIZE 100
#define INCORRECT_IDENTIFIER 5
#define KEY_EXISTS 6
#define HELPER_OUT_FILE "f_out_helper.c"
#define WR "w+"
#define CURSOR_UNMOVED 7

typedef struct {
	void* key;
	void* value;
} Pair;

#include "./DoubleLinkedList/DoubleLinkedList.h"
#include "./HashTable/HashTable.h"
#include "./Stack/Stack.h"

DoubleLinkedList* list = NULL;
HashTable* hash_table = NULL;
Stack* stack = NULL;

char in_file_name[NAME_SIZE];
char out_file_name[NAME_SIZE];
FILE* file_in = NULL;
FILE* file_out = NULL;

#endif // UTILS__H