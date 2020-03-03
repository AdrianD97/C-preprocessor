#include "utils_.h"
#include "parse.h"
#include "functions/compare_functions.h"
#include "functions/hash_functions.h"
#include "free_memory/free_memory.h"
#include "directives/directives.h"

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

void close_in_and_out_files(FILE* f_out) {
	int result;

	result = fclose(file_in);

	if (result) {
		printf("Error: Can not close %s file.\n", in_file_name);
	}

	result = fclose(file_out);
	if (result) {
		printf("Error: Can not close %s file.\n", out_file_name);
	}

	result = fclose(f_out);
	if (result) {
		printf("Error: Can not close %s file.\n", HELPER_OUT_FILE);
	}

	if (remove(HELPER_OUT_FILE)) {
		printf("Error: Can not remove %s file\n", HELPER_OUT_FILE);
	}
}

int directives_preprocessing(FILE* f_in, FILE* f_out) {
	int result, i;
	char* line, *copy_line;
	char words[LINE_WORDS][WORD_SIZE];
	int nr_words;
	int len;
	char* delm = "\t ";

	line = (char*)malloc(LINE_SIZE * sizeof(char));
	if (!line) {
		printf("Error: Can not alloc memory for \'line\' variable.\n");
		return ENOMEM;
	}

	copy_line = (char*)malloc(LINE_SIZE * sizeof(char));
	if (!copy_line) {
		printf("Error: Can not alloc memory for \'copy_line\' variable.\n");
		return ENOMEM;
	}

	while (fgets(line, LINE_SIZE, f_in) != NULL)  {
		len = strlen(line);
		if (line[len - 1] == '\n') {
			if (len > 1 && line[len - 2] == '\r') {
				--len;
			} 
			line[len - 1] = '\0';
		}
	    
	    strcpy(copy_line, line);
		split_line(line, words, &nr_words, delm);
		if (nr_words == 0) {
			fprintf(f_out, "\n");
			continue;
		}

		if (strcmp(words[0], "#define") == 0) {
			result = define_directive(f_in, f_out, words, nr_words);
			if (result != SUCCESS) {
				free(line);
				free(copy_line);
				return result;
			}
		} else if (strcmp(words[0], "#include") == 0) {
			// TODO: find file

			printf("%s\n", words[1]);
		} else {
			// TODO: Daca nu este nici-un fel de directiva, atunci scrie direct in fisierul intermediar linia
			fprintf(f_out, "%s\n", copy_line);
		}
	}

	// TODO: Close input file after preprocessing
	free(line);
	free(copy_line);

	return SUCCESS;
}

void get_string(char* word, char words[][WORD_SIZE], int nr_words, int* index) {
	int len;

	for (; *index < nr_words; ++*index) {
		if (strchr(words[*index], '\"')) {
			strcat(word, words[*index]);
			break;
		}
		
		strcat(word, words[*index]);
		strcat(word, " ");
	}
}

int get_symbol_final_value(char* value, char* final_value) {
	char words[LINE_WORDS][WORD_SIZE];
	int nr_words;
	int i, result;
	void* val;
	char* copy_val, *f_val;
	char* delm = "\t ";

	if (value[0] == '\0') {
		final_value[0] = '\0';
		return SUCCESS;
	}

	if (value[0] == '?') {
		strcpy(final_value, value + 1);
		return SUCCESS;
	}

	copy_val = (char*)malloc((strlen(value) + 1) * sizeof(char));
	if (!copy_val) {
		printf("Error: Can not alloc memory for \'copy_val\' variable.\n");
		return ENOMEM;
	}	

	strcpy(copy_val, value);

	split_line(copy_val, words, &nr_words, delm);
	if (nr_words == 0) {
		final_value[0] = '\0';
	}

	for (i = 0; i < nr_words; ++i) {
		if (strchr(words[i], '\"')) {
			strcat(final_value, words[i]);
			strcat(final_value, " ");
			++i;
			get_string(final_value, words, nr_words, &i);
			strcat(final_value, " ");
		} else if (is_identifier(words[i])) {
			f_val = (char*)malloc(MAPPING_SIZE * sizeof(char));
			if (!f_val) {
				printf("Error: Can not alloc memory for \'f_val\' variable.\n");
				free(copy_val);
				return ENOMEM;
			}
			f_val[0] = '\0';

			val = get(hash_table, (void*)words[i]);
			if (!val) {
				strcat(f_val, words[i]);
			} else {
				result = get_symbol_final_value(val, f_val);
				if (result != SUCCESS) {
					free(f_val);
					free(copy_val);
					return result;
				}
			}

			strcat(final_value, f_val);
			free(f_val);
		} else {
			strcat(final_value, words[i]);
			strcat(final_value, " ");
		}
	}

	free(copy_val);
	return SUCCESS;
}

int get_symbols_final_value() {
	int i;
	ListNode* node;
	Pair* pair;
	void* value;
	char* final_value;
	int result;

	for (i = 0; i < hash_table->size; ++i) {
		node = hash_table->map[i]->head;
		while (node != NULL) {
			pair = (Pair*)node->value;
			value = pair->value;

			final_value = (char*)malloc(MAPPING_SIZE * sizeof(char));
			if (!final_value) {
				printf("Error: Can not alloc memory for \'final_value\' variable.\n");
				return ENOMEM;
			}

			final_value[0] = '\0';

			result = get_symbol_final_value((char*)value, final_value);
			if (result != SUCCESS) {
				free(final_value);
				return result;
			}

			free(value);
			
			result = put(hash_table, pair->key, (void*)final_value);
			if (result != SUCCESS) {
				free(final_value);
				return result;
			}

			node = node->next;
		}
	}

	return SUCCESS;
}

int file_preprocessing(FILE* f_in) {
	char* line;
	int len, i;
	char words[LINE_WORDS][WORD_SIZE];
	int nr_words;
	char* final_line;
	void* val;
	char* delm = "\t ();";

	line = (char*)malloc(LINE_SIZE * sizeof(char));
	if (!line) {
		printf("Error: Can alloc memory for \'line\' variable.\n");
		return ENOMEM;
	}

	final_line = (char*)malloc(MAPPING_SIZE * sizeof(char));
	if (!final_line) {
		printf("Error: Can not alloc memory for \'final_line\' variable.\n");
		return ENOMEM;
	}


	while (fgets(line, LINE_SIZE, f_in) != NULL)  {
		len = strlen(line);
		if (line[len - 1] == '\n') {
			line[len - 1] = '\0';
		}

		if (line[0] == '\0') {
			continue;
		}

		split_line(line, words, &nr_words, delm);

		final_line[0] = '\0';

		for (i = 0; i < nr_words; ++i) {
			if (strchr(words[i], '\"')) {
				strcat(final_line, words[i]);
				strcat(final_line, " ");
				++i;
				get_string(final_line, words, nr_words, &i);
			} else if (is_identifier(words[i])) {
				val = get(hash_table, (void*)words[i]);
				if (!val) {
					strcat(final_line, words[i]);
				} else {
					strcat(final_line, val);
				}
			} else {
				strcat(final_line, words[i]);
			}
			strcat(final_line, " ");
		}

		fprintf(file_out, "%s\n", final_line);
	}

	free(line);
	free(final_line);
	return SUCCESS;
}

int main(int argc, char **argv) {
	int result;
	FILE* f_out_helper;

	f_out_helper = fopen(HELPER_OUT_FILE, WR);
	if (!f_out_helper) {
		printf("Error: Can not open %s file.\n", HELPER_OUT_FILE);
		return INVALID_FILE;
	}

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
	result = directives_preprocessing(file_in, f_out_helper);
	if (result != SUCCESS) {
		free_memory();
		close_in_and_out_files(f_out_helper);
		return result;
	}

	result = get_symbols_final_value();
	if (result != SUCCESS) {
		free_memory();
		close_in_and_out_files(f_out_helper);
		return result;
	}

	result = fseek(f_out_helper, 0, SEEK_SET);
	if (result) {
		free_memory();
		close_in_and_out_files(f_out_helper);
		printf("Error: can not move the cursor.\n");
		return CURSOR_UNMOVED;
	}

	result = file_preprocessing(f_out_helper);
	if (result != SUCCESS) {
		free_memory();
		close_in_and_out_files(f_out_helper);
		return result;
	}

	free_memory();
	close_in_and_out_files(f_out_helper);

	return 0;
}