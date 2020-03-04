#include "utils_.h"
#include "parse.h"
#include "functions/compare_functions.h"
#include "functions/hash_functions.h"
#include "free_memory/free_memory.h"
#include "directives/directives.h"
/*
	Vezi test 9
	se inlocuiesc toate aparitiile pana la undef 
	deci nasol, trebuie schimbata solutia cu fisier intermediar
	FUCK
*/

int init(int argc, char **argv)
{
	int result;
	char *path;
	int len;

	// create an empty list
	result = createEmptyDoubleLinkedList(&list, compare_strings);
	if (result != SUCCESS)
		return result;

	// create an empty hash table
	result = createEmptyHashTable(&hash_table, HASH_TABLE_SIZE,
		string_hash, compare_pairs_with_string_key);
	if (result != SUCCESS) {
		return result;
	}

	// create an empty stack(will contains open header files)
	result = createEmptyStack(&files, STACK_SIZE);
	if (result != SUCCESS) {
		return result;
	}

	result = parse_cmd_args(argc, argv);
	if (result != SUCCESS) {
		return result;
	}

	len = strlen(in_file_name);
	path = (char *)malloc(len * sizeof(char));
	if (!path) {
		printf("Error: Can not get file input path.\n");
		return -ENOMEM;
	}

	get_file_input_path(path, in_file_name, len);

	result = addItemToBeginningOfTheDoubleLinkedList(list, (void *)path);

	return result;
}

int open_in_and_out_files(void)
{
	int result;

	if (strcmp(in_file_name, STDIN) == 0) {
		file_in = stdin;
	} else {
		file_in = fopen(in_file_name, O_READ);
		if (!file_in) {
			printf("Error: Can not open %s file.\n", in_file_name);
			return INVALID_FILE;
		}
	}
	result = push(files, (void *)file_in);
	if (result != SUCCESS) {
		return result;
	}

	if (strcmp(out_file_name, STDOUT) == 0) {
		file_out = stdout;
	} else {
		file_out = fopen(out_file_name, O_WRITE);
		if (!file_out) {
			printf("Error: Can not "
				"open/create %s file.\n", out_file_name);
			return INVALID_FILE;
		}
	}

	return SUCCESS;
}

void close_out_helper_out_and_err_files(FILE *f_out)
{
	int result;

	result = fclose(file_out);
	if (result) {
		printf("Error: Can not close %s file.\n", out_file_name);
	}

	result = fclose(err_file);
	if (result) {
		printf("Error: Can not close %s file.\n", ERR_FILE);
	}

	result = fclose(f_out);
	if (result) {
		printf("Error: Can not close %s file.\n", HELPER_OUT_FILE);
	}

	if (remove(HELPER_OUT_FILE)) {
		printf("Error: Can not remove %s file\n", HELPER_OUT_FILE);
	}
}

int ignore_lines(FILE *f_in, FILE *f_out, char* line,
	Stack *stack, int *found, int ign)
{
	int len, result;
	char words[LINE_WORDS][WORD_SIZE];
	int nr_words;
	StackNode *node;
	char *delm = "\t ";
	*found = 0;

	while (fgets(line, LINE_SIZE, f_in) != NULL) {
		len = strlen(line);
		if (line[len - 1] == '\n') {
			if (len > 1 && line[len - 2] == '\r') {
				--len;
			}
			line[len - 1] = '\0';
		}

		split_line(line, words, &nr_words, delm);
		if (nr_words == 0) {
			fprintf(f_out, "\n");
			continue;
		}

		if (strcmp(words[0], "#if") == 0 ||
			strcmp(words[0], "#ifdef") == 0 ||
			strcmp(words[0], "#ifndef") == 0) {
			result = push(stack, (void *)_if);
			if (result != SUCCESS) {
				return result;
			}
		} else if (strcmp(words[0], "#endif") == 0) {
			node = pop(stack);
			free(node);
			if (stack->length == 0) {
				break;
			}
		} else if (!ign && (strcmp(words[0], "#else") == 0 || strcmp(words[0], "#elif") == 0)) {
			result = fseek(f_in, -len, SEEK_CUR);
			*found = 1;
			if (result) {
				printf("Error: can not move the cursor.\n");
				return CURSOR_UNMOVED;
			}
			break;
		}

		// TODO:
		/*
			Daca intalnesc #elif sau #else ma opresc si ma intorc in directives_preprocessing(
			pentru elif, va trebui sa ma intorc cu o linie inapoi deoarece trebuie sa verific daca
			cond este true sau nu)
		*/
	}

	// TODO:
	// Daca am citit tot fisierul si nu am gasit #endif => eroare
	return SUCCESS;
}

int directives_preprocessing(FILE *f_in, FILE *f_out, int ign)
{
	int result;
	char *line, *copy_line;
	char words[LINE_WORDS][WORD_SIZE];
	int nr_words;
	int len;
	char *delm = "\t ";
	ListNode *node;
	char *file_path;
	char file_name[NAME_SIZE];
	void *val;
	Stack *stack;
	int found;

	line = (char *)malloc(LINE_SIZE * sizeof(char));
	if (!line) {
		printf("Error: Can not alloc memory for \'line\' variable.\n");
		return -ENOMEM;
	}

	copy_line = (char *)malloc(LINE_SIZE * sizeof(char));
	if (!copy_line) {
		printf("Error: Can not alloc memory "
			"for \'copy_line\' variable.\n");
		return -ENOMEM;
	}

	while (fgets(line, LINE_SIZE, f_in) != NULL) {
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
			FILE *f;
			int found = 0;

			if (words[1][0] == '<') {
				continue;
			}
			len = strlen(words[1]);
			words[1][len - 1] = '\0';
			strcpy(file_name, words[1] + 1);

			file_path = (char *)malloc(PATH_SIZE * sizeof(char));
			if (!file_path) {
				printf("Error: Can not alloc memory for \'file_path\' variable.\n");
				free(line);
				free(copy_line);
				return -ENOMEM;
			}

			node = list->head;
			while (node != NULL) {
				strcpy(file_path, (char *)node->value);
				len = strlen(file_path);

				if (file_path[len - 1] != '/') {
					strcat(file_path, "/");
				}
				strcat(file_path, file_name);

				f = fopen(file_path, O_READ);
				if (f) {
					found = 1;
					result = push(files, (void *)f);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						return result;
					}
					break;
				}

				node = node->next;
			}

			free(file_path);
			if (!found) {
				printf("Error: File %s doesn\'t "
					"exists.\n", file_name);
				free(line);
				free(copy_line);
				return INVALID_FILE;
			}
			result = directives_preprocessing(f, f_out, 0);

			if (result != SUCCESS) {
				free(line);
				free(copy_line);
				return result;
			}
		} else if (strcmp(words[0], "#ifdef") == 0) {
			// In loc de ifdef pun if, si mai trebuie sa adaug pe langa identifier sa verific ca este
			// numar si ca este diferit de 0
			// TODO: Defpat nu schimb ci doar copiez acest ifdef pt if, si mai verific ca este numar diferit de 0
			// sau valoarea literalui sa fie diferita de 0
			/*
			OBS asta trebuie si pentru elif
				1. daca este numar, trebuie sa fie diferit de 0
				2. daca este symbol, trebuie sa existe si sa aiba valoare diferita de 0.
				3. orice altceva insemana false
			*/
			if (!is_identifier(words[1])) {
				printf("Error: %s is invalid identifier.\n", words[1]);
				free(line);
				free(copy_line);
				return INVALID_IDENTIFIER;
			}


			val = get(hash_table, (void *)words[1]);
			if (!val) {
				result = createEmptyStack(&stack, STACK_SIZE);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					return result;
				}
				
				result = push(stack, (void *)_if);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				result = ignore_lines(f_in, f_out, line, stack, &found, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);

				if (found) {
					result = directives_preprocessing(f_in, f_out, 0);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
					}
				}
			} else {
				result = directives_preprocessing(f_in, f_out, 1);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
				}
			}
		} else if (strcmp(words[0], "#ifndef") == 0) {
			if (!is_identifier(words[1])) {
				printf("Error: %s is invalid identifier.\n", words[1]);
				free(line);
				free(copy_line);
				return INVALID_IDENTIFIER;
			}


			val = get(hash_table, (void *)words[1]);
			if (val) {
				result = createEmptyStack(&stack, STACK_SIZE);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					return result;
				}
				
				result = push(stack, (void *)_if);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				result = ignore_lines(f_in, f_out, line, stack, &found, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);

				if (found) {
					result = directives_preprocessing(f_in, f_out, 0);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
					}
				}
			} else {
				result = directives_preprocessing(f_in, f_out, 1);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
				}
			}
		} else if (strcmp(words[0], "#if") == 0) {
			int is_n_lit = is_number_literal(words[1]);
			int is_id = is_identifier(words[1]);
			int process_lines = 0;

			if (!is_n_lit && !is_id) {
				printf("Error: %s is invalid identifier/number_literal.\n", words[1]);
				free(line);
				free(copy_line);
				return INVALID_IDENTIFIER;
			}

			if (is_n_lit && words[1][0] != '0') {
				process_lines = 1;
			}

			if (!process_lines && is_id) {
				val = get(hash_table, (void *)words[1]);
				if (val) {
					if (((char*)val)[0] != '0') {
						process_lines = 1;
					}
				}
			}

			if (!process_lines) {
				result = createEmptyStack(&stack, STACK_SIZE);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					return result;
				}
				
				result = push(stack, (void *)_if);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				result = ignore_lines(f_in, f_out, line, stack, &found, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);

				if (found) {
					result = directives_preprocessing(f_in, f_out, 0);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
					}
				}
			} else {
				result = directives_preprocessing(f_in, f_out, 1);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
				}
			}
		} else if (strcmp(words[0], "#elif") == 0) {
			if (ign == 1) {
				result = createEmptyStack(&stack, STACK_SIZE);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					return result;
				}
				
				result = push(stack, (void *)_if);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				result = ignore_lines(f_in, f_out, line, stack, &found, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);

				if (found) {
					result = directives_preprocessing(f_in, f_out, 1);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
					}
				}
			} else {
				int is_n_lit = is_number_literal(words[1]);
				int is_id = is_identifier(words[1]);
				int process_lines = 0;

				if (!is_n_lit && !is_id) {
					printf("Error: %s is invalid identifier/number_literal.\n", words[1]);
					free(line);
					free(copy_line);
					return INVALID_IDENTIFIER;
				}

				if (is_n_lit && words[1][0] != '0') {
					process_lines = 1;
				}

				if (!process_lines && is_id) {
					val = get(hash_table, (void *)words[1]);
					if (val) {
						if (((char*)val)[0] != '0') {
							process_lines = 1;
						}
					}
				}

				if (!process_lines) {
					result = createEmptyStack(&stack, STACK_SIZE);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						return result;
					}
					
					result = push(stack, (void *)_if);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						free_stack_memory(&stack);
						return result;
					}

					result = ignore_lines(f_in, f_out, line, stack, &found, 0);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						free_stack_memory(&stack);
						return result;
					}

					free_stack_memory(&stack);

					if (found) {
						result = directives_preprocessing(f_in, f_out, 0);
						if (result != SUCCESS) {
							free(line);
							free(copy_line);
						}
					}
				} else {
					result = directives_preprocessing(f_in, f_out, 1);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
					}
				}
			}
		} else if (strcmp(words[0], "#else") == 0) {
			if (ign == 1) {
				result = createEmptyStack(&stack, STACK_SIZE);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					return result;
				}
				
				result = push(stack, (void *)_if);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				result = ignore_lines(f_in, f_out, line, stack, &found, 1);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);
			} else {
				result = directives_preprocessing(f_in, f_out, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
				}
			}
				
		} else if (strcmp(words[0], "#undef") == 0) {
			if (!is_identifier(words[1])) {
				printf("Error: %s is invalid identifier.\n", words[1]);
				free(line);
				free(copy_line);
				return INVALID_IDENTIFIER;
			}

			result = erase(hash_table, (void *)words[1]);
			if (result != SUCCESS) {
				printf("Error: Can not remove %s identifier\n", words[1]);
				free(line);
				free(copy_line);
				return result;
			}
		} else if (strcmp(words[0], "#endif") != 0) {
			fprintf(f_out, "%s\n", copy_line);
		}
	}

	free(line);
	free(copy_line);

	return SUCCESS;
}

void get_string(char *word, char words[][WORD_SIZE],
	int nr_words, int *index)
{
	for (; *index < nr_words; ++*index) {
		if (strchr(words[*index], '\"')) {
			strcat(word, words[*index]);
			break;
		}
		strcat(word, words[*index]);
		strcat(word, " ");
	}
}

int get_symbol_final_value(char *value, char *final_value)
{
	char words[LINE_WORDS][WORD_SIZE];
	int nr_words;
	int i, result;
	void *val;
	char *copy_val, *f_val;
	char *delm = "\t ";

	if (value[0] == '\0') {
		final_value[0] = '\0';
		return SUCCESS;
	}

	if (value[0] == '?') {
		strcpy(final_value, value + 1);
		return SUCCESS;
	}

	copy_val = (char *)malloc((strlen(value) + 1) * sizeof(char));
	if (!copy_val) {
		printf("Error: Can not alloc memory "
			"for \'copy_val\' variable.\n");
		return -ENOMEM;
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
			f_val = (char *)malloc(MAPPING_SIZE * sizeof(char));
			if (!f_val) {
				printf("Error: Can not alloc memory "
					"for \'f_val\' variable.\n");
				free(copy_val);
				return -ENOMEM;
			}
			f_val[0] = '\0';

			val = get(hash_table, (void *)words[i]);
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

int get_symbols_final_value(void)
{
	int i;
	ListNode *node;
	Pair *pair;
	void *value;
	char *final_value;
	int result;

	for (i = 0; i < hash_table->size; ++i) {
		node = hash_table->map[i]->head;
		while (node != NULL) {
			pair = (Pair *)node->value;
			value = pair->value;

			final_value = (char *)malloc(MAPPING_SIZE * sizeof(char));
			if (!final_value) {
				printf("Error: Can not alloc memory "
					"for \'final_value\' variable.\n");
				return -ENOMEM;
			}

			final_value[0] = '\0';

			result = get_symbol_final_value((char *)value, final_value);
			if (result != SUCCESS) {
				free(final_value);
				return result;
			}

			free(value);

			result = put(hash_table, pair->key, (void *)final_value);
			if (result != SUCCESS) {
				free(final_value);
				return result;
			}

			node = node->next;
		}
	}

	return SUCCESS;
}

int file_preprocessing(FILE *f_in)
{
	char *line;
	int len, i;
	char words[LINE_WORDS][WORD_SIZE];
	int nr_words;
	char *final_line;
	void *val;
	char *delm = "\t ();";
	char *occ;

	line = (char *)malloc(LINE_SIZE * sizeof(char));
	if (!line) {
		printf("Error: Can not alloc memory for \'line\' variable.\n");
		return -ENOMEM;
	}

	final_line = (char *)malloc(MAPPING_SIZE * sizeof(char));
	if (!final_line) {
		printf("Error: Can not alloc memory for \'final_line\' variable.\n");
		return -ENOMEM;
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
			occ = strchr(words[i], '\"');
			if (occ) {
				strcat(final_line, words[i]);
				strcat(final_line, " ");
				if (strchr(occ, '\"')) {
					continue;
				}
				++i;
				get_string(final_line, words, nr_words, &i);
			} else if (is_identifier(words[i])) {
				val = get(hash_table, (void *)words[i]);
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

int main(int argc, char **argv)
{
	int result;
	FILE *f_out_helper;

	err_file = fopen(ERR_FILE, O_WRITE);
	if (!err_file) {
		printf("Error: Can not create err_file.\n");
		return -1;
	}

	result = init(argc, argv);
	if (result != SUCCESS) {
		free_memory();
		close_file(err_file);
		return result;
	}

	result = open_in_and_out_files();
	if (result != SUCCESS) {
		free_memory();
		close_file(err_file);
		return result;
	}

	f_out_helper = fopen(HELPER_OUT_FILE, WR);
	if (!f_out_helper) {
		printf("Error: Can not open %s file.\n", HELPER_OUT_FILE);
		close_file(file_in);
		close_file(file_out);
		close_file(err_file);
		return INVALID_FILE;
	}

	result = directives_preprocessing(file_in, f_out_helper, 0);

	if (result != SUCCESS) {
		free_memory();
		close_out_helper_out_and_err_files(f_out_helper);
		return result;
	}

	result = get_symbols_final_value();
	if (result != SUCCESS) {
		free_memory();
		close_out_helper_out_and_err_files(f_out_helper);

		return result;
	}

	result = fseek(f_out_helper, 0, SEEK_SET);
	if (result) {
		free_memory();
		close_out_helper_out_and_err_files(f_out_helper);

		printf("Error: can not move the cursor.\n");
		return CURSOR_UNMOVED;
	}

	result = file_preprocessing(f_out_helper);
	if (result != SUCCESS) {
		free_memory();
		close_out_helper_out_and_err_files(f_out_helper);

		return result;
	}

	free_memory();
	close_out_helper_out_and_err_files(f_out_helper);

	return 0;
}