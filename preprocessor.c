#include <assert.h>
#include "utils_.h"
#include "functions/compare_functions.h"
#include "functions/hash_functions.h"

// free_memory:
void free_used_memory() {
	freeDoubleLinkedListMemory(list);
	free(list);
	list = NULL;

	freeHashTableMemory(hash_table);
	free(hash_table);
	hash_table = NULL;

	freeStackMemory(stack);
	free(stack);
	stack = NULL;
}

int main(int argc, char const *argv[]) {
	int result;

	assert(list == NULL);
	result = createEmptyDoubleLinkedList(&list, comparePairsWithStringKey);
	assert(result == SUCCESS);
	assert(list != NULL);

	char* key1, *key2;
	int val1 = 90, val2 = 89;
	key1 = (char*)malloc(10 * sizeof(char));
	strcpy(key1, "Ana");
	key2 = (char*)malloc(10 * sizeof(char));
	strcpy(key2, "Mihai");

	// test DoubleLinkedList
	result = addItemToDoubleLinkedList(list, (void*)key1);
	assert(result == SUCCESS);
	assert(list->length == 1);
	result = addItemToDoubleLinkedList(list, (void*)key2);
	assert(result == SUCCESS);
	assert(list->length == 2);

	// test HashTable
	assert(hash_table == NULL);
	result = createEmptyHashTable(&hash_table, 100, stringHash, comparePairsWithStringKey);
	assert(result == SUCCESS);
	assert(hash_table != NULL);

	void* v;
	result = put(hash_table, (void*)key1, (void*)&val1);
	assert(result == SUCCESS);
	v = get(hash_table, (void*)key1);
	assert(*(int*)v == val1);

	result = put(hash_table, (void*)key2, (void*)&val2);
	assert(result == SUCCESS);
	v = get(hash_table, (void*)key2);
	assert(*(int*)v == val2);

	result = put(hash_table, (void*)key1, (void*)&val2);
	assert(result == SUCCESS);
	v = get(hash_table, (void*)key1);
	assert(*(int*)v == val2);

	result = erase(hash_table, (void*)key1);
	assert(result == SUCCESS);
	v = get(hash_table, (void*)key1);
	assert(v == NULL);

	// test Stack
	assert(stack == NULL);
	result = createEmptyStack(&stack, 20);
	assert(result == SUCCESS);
	assert(stack != NULL);
	result = push(stack, (void*)&val1);
	assert(result == SUCCESS);
	result = push(stack, (void*)&val2);
	assert(result == SUCCESS);

	StackNode* node = pop(stack);
	assert(node != NULL);
	assert(*(int*)node->value == val2);

	free(node);
	free(key1);
	free(key2);

	free_used_memory();
	return 0;
}