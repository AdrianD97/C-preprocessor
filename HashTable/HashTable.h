#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>

#include "utils_hash_table/utils_hash_table.h"

// create an empty hash table
int createEmptyHashTable(HashTable **hash_table, int size,
	unsigned int (*hash)(void *), unsigned int (*compare)(void *, void *))
{
	int result, i;

	if (!hash_table) {
		printf("Error: Param \'hash_table\' has invalid value.\n");
		return INVALID_VALUE;
	}

	*hash_table = (HashTable *)malloc(sizeof(HashTable));

	if (!(*hash_table)) {
		printf("Error: Can not alloc memory for hash_table object.\n");
		return -ENOMEM;
	}

	(*hash_table)->size = size;
	(*hash_table)->hash = hash;
	(*hash_table)->map = (DoubleLinkedList **)malloc(size * sizeof(DoubleLinkedList *));

	if (!(*hash_table)->map) {
		printf("Error: Can not alloc memory for hash_table internal map.\n");
		return -ENOMEM;
	}

	for (i = 0; i < size; ++i) {
		result = createEmptyDoubleLinkedList(&((*hash_table)->map[i]), compare);
		if (result != SUCCESS) {
			return result;
		}
	}

	return SUCCESS;
}

// add an item to hash table
int put(HashTable *hash_table, void *key, void *value)
{
	int index = hash_table->hash(key) % hash_table->size;
	int result;
	ListNode *node;
	Pair *pair;

	pair = (Pair *)malloc(sizeof(Pair));
	if (!pair) {
		printf("Error: Can not add a new item to hash table.\n");
		return -ENOMEM;
	}
	pair->key = key;
	pair->value = value;

	node = findElementByValue(hash_table->map[index], (void *)pair);
	if (node) {
		Pair *help = (Pair *)node->value;
		help->value = value;
		free(pair);

		result = SUCCESS;
	} else {
		result = addItemToDoubleLinkedList(hash_table->map[index], (void *)pair);
	}

	return result;
}

// get value associated with the given key
void *get(const HashTable *hash_table, void *key)
{
	int index = hash_table->hash(key) % hash_table->size;
	ListNode *node;

	Pair *pair, *pair_help;

	pair = (Pair *)malloc(sizeof(Pair));
	pair->key = key;
	pair->value = NULL;

	node = findElementByValue(hash_table->map[index], (void *)pair);

	free(pair);
	if (!node) {
		return NULL;
	}

	pair_help = (Pair *)node->value;

	return pair_help->value;
}

// remove a (key, value) pair from hash table
int erase(HashTable *hash_table, void *key)
{
	int index = hash_table->hash(key) % hash_table->size;
	ListNode *node;
	Pair *pair;

	pair = (Pair *)malloc(sizeof(Pair));
	if (!pair) {
		printf("Error: Can not remove item from hash table.\n");
		return -ENOMEM;
	}
	pair->key = key;
	pair->value = NULL;

	node = removeItemFromDoubleLinkedList(hash_table->map[index], (void *)pair);
	if (node) {
		free(node->value);
		free(node);
	}

	free(pair);

	return SUCCESS;
}

// free used memory
void freeHashTableMemory(HashTable *hash_table)
{
	unsigned int i;
	ListNode *node;

	if (!hash_table) {
		return;
	}
	for (i = 0; i < hash_table->size; ++i) {
		node = hash_table->map[i]->head;
		while (node != NULL) {
			free(node->value);
			node = node->next;
		}

		freeDoubleLinkedListMemory(hash_table->map[i]);
		free(hash_table->map[i]);
	}

	free(hash_table->map);
	hash_table->map = NULL;
}

#endif // HASH_TABLE_H
