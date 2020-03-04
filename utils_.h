#ifndef UTILS__H
#define UTILS__H

#define ENOMEM -12
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
#define INVALID_IDENTIFIER 5
#define KEY_EXISTS 6
#define WR "w+"
#define CURSOR_UNMOVED 7
#define PATH_SIZE 256
#define NR_HEADERS_FILES 100
#define ERR_FILE "err_file.log"
#define UNTERMINATED_IF 8

typedef struct {
	void *key;
	void *value;
} Pair;

#include "./DoubleLinkedList/DoubleLinkedList.h"
#include "./HashTable/HashTable.h"
#include "./Stack/Stack.h"


DoubleLinkedList * list;
HashTable *hash_table;
Stack *files;

char in_file_name[NAME_SIZE];
char out_file_name[NAME_SIZE];
FILE *file_in;
FILE *file_out;
FILE *err_file;

char *_if = "if*";

#endif // UTILS__H
