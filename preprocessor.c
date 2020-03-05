#include "utils_.h"
#include "parse.h"
#include "functions/compare_functions.h"
#include "functions/hash_functions.h"
#include "free_memory/free_memory.h"
#include "check_symbol_type.h"
#include "symbols_values.h"
#include "directives/directives.h"

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
	if (result != SUCCESS)
		return result;

	// create an empty stack(will contains open header files)
	result = createEmptyStack(&files, STACK_SIZE);
	if (result != SUCCESS)
		return result;

	result = parse_cmd_args(argc, argv);
	if (result != SUCCESS)
		return result;

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
	if (result != SUCCESS)
		return result;

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

int ignore_lines(FILE *f_in, FILE *f_out, char *line,
	Stack *stack, int *found, int ign)
{
	int len, result;
	char words[LINE_WORDS][WORD_SIZE];
	int nr_words;
	StackNode *node;
	char *delm = "\t ";
	int end_of_file = 1;

	*found = 0;

	while (fgets(line, LINE_SIZE, f_in) != NULL) {
		len = strlen(line);
		if (line[len - 1] == '\n') {
			if (len > 1 && line[len - 2] == '\r')
				--len;

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
			if (result != SUCCESS)
				return result;

		} else if (strcmp(words[0], "#endif") == 0) {
			node = pop(stack);
			free(node);
			if (stack->length == 0) {
				end_of_file = 0;
				break;
			}
		} else if (!ign && (strcmp(words[0], "#else") == 0
			|| strcmp(words[0], "#elif") == 0)) {
			result = fseek(f_in, -(len + 1), SEEK_CUR);
			*found = 1;
			if (result) {
				printf("Error: can not move the cursor.\n");
				return CURSOR_UNMOVED;
			}
			end_of_file = 0;
			break;
		}
	}

	if (end_of_file) {
		printf("Error: unterminated #if*\n");
		return UNTERMINATED_IF;
	}
	return SUCCESS;
}

void get_delms(char *delms, char *line, int pos)
{
	char *str = " \t[]{}<>=+-*/%!&|^.,:;()\\";
	char *ret;
	char ch;
	int j = 0;
	int l, r;

	ret = strchr(str, line[pos]);

	while (pos >= 0 && ret) {
		delms[j++] = line[pos];
		--pos;
		if (pos >= 0)
			ret = strchr(str, line[pos]);

	}

	delms[j] = '\0';

	l = 0;
	r = j - 1;

	while (l <= r) {
		ch = delms[l];
		delms[l] = delms[r];
		delms[r] = ch;
		++l;
		--r;
	}
}

int get_final_line(char words[][WORD_SIZE], int nr_words,
	char *line, char *final_line)
{
	int i, pos, result;
	char val[MAPPING_SIZE];
	char delms[50];
	char *occ = line;

	for (i = 0; i < nr_words; ++i) {
		occ = strstr(occ, words[i]);
		occ += 1;
		strcpy(delms, "");
		if (occ) {
			pos = occ - line;
			pos -= 2;
			if (pos >= 0)
				get_delms(delms, line, pos);

			strcat(final_line, delms);
		}

		if (is_var_type(words[i])) {
			strcat(final_line, words[i]);
		} else if (is_identifier(words[i])) {
			strcpy(val, "");
			result = get_symbol_value(words[i], val);
			if (result != SUCCESS)
				return result;

			val[strlen(val) - 1] = '\0';
			strcat(final_line, val);
		} else {
			strcat(final_line, words[i]);
		}
	}


	// printf("final_line = |%s|\n", final_line);
	get_delms(delms, line, strlen(line) - 1);
	strcat(final_line, delms);

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
	char value_[MAPPING_SIZE];

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
			if (len > 1 && line[len - 2] == '\r')
				--len;

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

			if (words[1][0] == '<')
				continue;

			len = strlen(words[1]);
			words[1][len - 1] = '\0';
			strcpy(file_name, words[1] + 1);

			file_path = (char *)malloc(PATH_SIZE * sizeof(char));
			if (!file_path) {
				printf("Error: Can not alloc "
					"memory for \'file_path\' variable.\n");
				free(line);
				free(copy_line);
				return -ENOMEM;
			}

			node = list->head;
			while (node != NULL) {
				strcpy(file_path, (char *)node->value);
				len = strlen(file_path);

				if (file_path[len - 1] != '/')
					strcat(file_path, "/");

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
			if (!is_identifier(words[1])) {
				printf("Error: %s is invalid "
					"identifier.\n", words[1]);
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

				result = ignore_lines(f_in, f_out,
					line, stack, &found, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);

				if (found) {
					result = directives_preprocessing(f_in,
						f_out, 0);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						return result;
					}
				}
			} else {
				result = directives_preprocessing(f_in,
					f_out, 1);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					return result;
				}
			}
		} else if (strcmp(words[0], "#ifndef") == 0) {
			if (!is_identifier(words[1])) {
				printf("Error: %s is invalid "
					"identifier.\n", words[1]);
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

				result = ignore_lines(f_in, f_out, line,
					stack, &found, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);

				if (found) {
					result = directives_preprocessing(f_in,
						f_out, 0);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						return result;
					}
				}
			} else {
				result = directives_preprocessing(f_in,
					f_out, 1);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					return result;
				}
			}
		} else if (strcmp(words[0], "#if") == 0) {
			int is_n_lit = is_number_literal(words[1]);
			int is_id = is_identifier(words[1]);
			int process_lines = 0;

			if (!is_n_lit && !is_id) {
				printf("Error: %s is invalid "
					"identifier/number_literal.\n",
					words[1]);
				free(line);
				free(copy_line);
				return INVALID_IDENTIFIER;
			}

			if (is_n_lit && words[1][0] != '0')
				process_lines = 1;

			if (!process_lines && is_id) {
				val = get(hash_table, (void *)words[1]);
				if (val) {
					if (((char *)val)[0] == '\0') {
						printf("Error: #if with "
							"no expression.\n");
						free(line);
						free(copy_line);
						return INVALID_VALUE;
					}

					strcpy(value_, "");
					result = get_symbol_value((char *)val,
						value_);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						return result;
					}

					value_[strlen(value_) - 1] = '\0';
					if (value_[0] != '0')
						process_lines = 1;
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

				result = ignore_lines(f_in,
					f_out, line, stack, &found, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);

				if (found) {
					result = directives_preprocessing(f_in,
						f_out, 0);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						return result;
					}
				}
			} else {
				result = directives_preprocessing(f_in,
					f_out, 1);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					return result;
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

				result = ignore_lines(f_in,
					f_out, line, stack, &found, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);

				if (found) {
					result = directives_preprocessing(f_in,
						f_out, 1);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						return result;
					}
				}
			} else {
				int is_n_lit = is_number_literal(words[1]);
				int is_id = is_identifier(words[1]);
				int process_lines = 0;

				if (!is_n_lit && !is_id) {
					printf("Error: %s is invalid "
						"identifier/number_literal.\n",
						words[1]);
					free(line);
					free(copy_line);
					return INVALID_IDENTIFIER;
				}

				if (is_n_lit && words[1][0] != '0')
					process_lines = 1;

				if (!process_lines && is_id) {
					val = get(hash_table, (void *)words[1]);
					if (val && ((char *)val)[0] != '0')
						process_lines = 1;
				}

				if (!process_lines) {
					result = createEmptyStack(&stack,
						STACK_SIZE);
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

					result = ignore_lines(f_in, f_out,
						line, stack, &found, 0);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						free_stack_memory(&stack);
						return result;
					}

					free_stack_memory(&stack);

					if (found) {
						result = directives_preprocessing(
								f_in, f_out, 0);
						if (result != SUCCESS) {
							free(line);
							free(copy_line);
							return result;
						}
					}
				} else {
					result = directives_preprocessing(f_in,
						f_out, 1);
					if (result != SUCCESS) {
						free(line);
						free(copy_line);
						return result;
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

				result = ignore_lines(f_in, f_out,
					line, stack, &found, 1);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					free_stack_memory(&stack);
					return result;
				}

				free_stack_memory(&stack);
			} else {
				result = directives_preprocessing(f_in,
					f_out, 0);
				if (result != SUCCESS) {
					free(line);
					free(copy_line);
					return result;
				}
			}

		} else if (strcmp(words[0], "#undef") == 0) {
			if (!is_identifier(words[1])) {
				printf("Error: %s is invalid "
					"identifier.\n", words[1]);
				free(line);
				free(copy_line);
				return INVALID_IDENTIFIER;
			}

			result = erase(hash_table, (void *)words[1]);
			if (result != SUCCESS) {
				printf("Error: Can not remove "
					"%s identifier\n", words[1]);
				free(line);
				free(copy_line);
				return result;
			}
		} else if (strcmp(words[0], "#endif") != 0) {

			char *delm_ = " \t[]{}<>=+-*/%!&|^.,:;()";
			char *final_line;

			strcpy(line, copy_line);
			split_line(line, words, &nr_words, delm_);

			final_line = (char *)malloc(LINE_SIZE * sizeof(char));
			if (!final_line) {
				printf("Error: Can not alloc "
					"memory for \'final_line\' variable.\n");
				free(line);
				free(copy_line);
				return -ENOMEM;
			}

			strcpy(final_line, "");
			result = get_final_line(words, nr_words,
				copy_line, final_line);
			if (result != SUCCESS) {
				free(line);
				free(copy_line);
				return result;
			}

			fprintf(f_out, "%s\n", final_line);
			free(final_line);
		}
	}

	free(line);
	free(copy_line);

	return SUCCESS;
}

int main(int argc, char **argv)
{
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

	result = directives_preprocessing(file_in, file_out, 0);

	if (result != SUCCESS) {
		free_memory();
		close_file(file_out);
		return result;
	}

	free_memory();
	close_file(file_out);

	return 0;
}
