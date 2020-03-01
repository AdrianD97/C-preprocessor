#ifndef FREE_MEMORY_H
#define FREE_MEMORY_H

void free_list_memory() {
	if (list) {
		freeDoubleLinkedListMemory(list);
		free(list);
		list = NULL;
	}
}

void free_hash_table_memory() {
	if (hash_table) {
		freeHashTableMemory(hash_table);
		free(hash_table);
		hash_table = NULL;
	}
}

void free_stack_memory() {
	if (stack) {
		freeStackMemory(stack);
		free(stack);
		stack = NULL;
	}
}

void free_data_structures_memory() {
	free_list_memory();
	free_hash_table_memory();
	free_stack_memory();
}

void free_data_structures_content_memory() {
	int i;
	ListNode* node;

	// free hash table data
	for (i = 0; i < hash_table->size; ++i) {
		node = hash_table->map[i]->head;
		while (node != NULL) {
			Pair* pair = (Pair*)node->value;
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
}

void free_memory() {
	free_data_structures_content_memory();
	free_data_structures_memory();
}

#endif // FREE_MEMORY_H