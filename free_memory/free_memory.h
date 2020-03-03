#ifndef FREE_MEMORY_H
#define FREE_MEMORY_H

void free_list_memory(void)
{
	if (list) {
		freeDoubleLinkedListMemory(list);
		free(list);
		list = NULL;
	}
}

void free_hash_table_memory(void)
{
	if (hash_table) {
		freeHashTableMemory(hash_table);
		free(hash_table);
		hash_table = NULL;
	}
}

void free_stack_memory(Stack **stack)
{
	if (*stack) {
		freeStackMemory(*stack);
		free(*stack);
		*stack = NULL;
	}
}

void free_data_structures_memory(void)
{
	free_list_memory();
	free_hash_table_memory();
	free_stack_memory(&stack);
	free_stack_memory(&files);
}

void free_data_structures_content_memory(void)
{
	int i;
	ListNode *node;
	Pair *pair;

	// free hash table data
	for (i = 0; i < hash_table->size; ++i) {
		node = hash_table->map[i]->head;
		while (node != NULL) {
			pair = (Pair *)node->value;
			// TODO: only testing purpose
			printf("(%s, %s)\n", (char *)pair->key, (char *)pair->value);
			/////////////////////
			free(pair->key);
			free(pair->value);

			node = node->next;
		}
	}

	// free list data
	node = list->head;
	while (node != NULL) {
		free(node->value);
		node = node->next;
	}

	// here free stack memory
	// pentru files nu trebuie pentru ca fac o functie care elibereaza memoria
}

void close_file(FILE *f)
{
	int result;

	result = fclose(f);
	if (result) {
		printf("Error: Can not close file.\n");
	}
}

void close_files(void)
{
	StackNode *node;
	while (files->length != 0) {
		node = pop(files);
		close_file((FILE *)node->value);
		free(node);
	}
}

void free_memory(void)
{
	close_files();
	free_data_structures_content_memory();
	free_data_structures_memory();
}

#endif // FREE_MEMORY_H
