#include "utils_.h"
#include "parse.h"
#include "functions/compare_functions.h"
#include "functions/hash_functions.h"
#include "free_memory/free_memory.h"

int init(int argc, char** argv) {
	int result;
	char* path;
	int len;

	// create an empty list
	result = createEmptyDoubleLinkedList(&list, compare_strings);
	if (result != SUCCESS) {
		return result;
	}

	// create an empty hash table
	result = createEmptyHashTable(&hash_table, HASH_TABLE_SIZE, string_hash, compare_pairs_with_string_key);
	if (result != SUCCESS) {
		return result;
	}

	// create an empty stack
	result = createEmptyStack(&stack, STACK_SIZE);
	if (result != SUCCESS) {
		return result;
	}

	result = parse_cmd_args(argc, argv);
	if (result != SUCCESS) {
		return result;
	}

	len = strlen(in_file_name);
	path = (char*)malloc(len * sizeof(char));
	if (!path) {
		printf("Error: Can not get file input path.\n");
		return ENOMEM;
	}

	get_file_input_path(path, in_file_name, len);

	result = addItemToBeginningOfTheDoubleLinkedList(list, (void*)path);

	return result;
}

int open_in_and_out_files() {
	if (strcmp(in_file_name, STDIN) == 0) {
		file_in = stdin;
	} else {
		file_in = fopen(in_file_name, O_READ);
		if (!file_in) {
			printf("Error: Can not open %s file.\n", in_file_name);
			return INVALID_FILE;
		}
	}

	if (strcmp(out_file_name, STDOUT) == 0) {
		file_out = stdout;
	} else {
		file_out = fopen(out_file_name, O_WRITE);
		if (!file_out) {
			printf("Error: Can not open/create %s file.\n", out_file_name);
			return INVALID_FILE;
		}
	}

	return SUCCESS;
}

void close_in_and_out_files() {
	int result;

	result = fclose(file_in);

	if (result) {
		printf("Error: Can not close %s file.\n", in_file_name);
	}

	result = fclose(file_out);
	if (result) {
		printf("Error: Can not close %s file.\n", out_file_name);
	}
}

int main(int argc, char **argv) {
	int result;

	result = init(argc, argv);
	if (result != SUCCESS) {
		free_memory();
		return result;
	}

	result = open_in_and_out_files();
	if (result != SUCCESS) {
		free_memory();
		return result;
	}

	// TODO: preprocesare fisieului de input si scrieerea rezultatului in fisierul de output

	free_memory();
	close_in_and_out_files();
	return 0;
}