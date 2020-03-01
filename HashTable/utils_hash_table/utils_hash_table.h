#ifndef UTILS_HASH_TABLE_H
#define UTILS_HASH_TABLE_H

#include "../../DoubleLinkedList/DoubleLinkedList.h"

typedef struct {
	int size;
	unsigned int (*hash)(void*);
	DoubleLinkedList** map;
} HashTable;

#endif // UTILS_HASH_TABLE_H