#ifndef UTILS__H
#define UTILS__H

#define ENOMEM 12
#define SUCCESS 0
#define INVALID_VALUE 1
#define FULL_STACK 2

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

#endif // UTILS__H