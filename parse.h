#ifndef PARSE_H
#define PARSE_H

#include <string.h>
#include <stdlib.h>

int parse_arg(char *arg)
{
	char *token;
	char *symbol;
	char *value;
	int len = 2, result;
	void *val;

	token = strtok(arg, "=");

	symbol = (char *)malloc((strlen(token) + 1) * sizeof(char));
	if (!symbol)
		return -ENOMEM;

	strcpy(symbol, token);

	token = strtok(NULL, "=");

	if (token)
		len = strlen(token) + 2;

	value = (char *)malloc(len * sizeof(char));
	if (!value) {
		free(symbol);
		return -ENOMEM;
	}

	if (!token) {
		value[0] = '\0';
	} else {
		strcpy(value, "?");
		strcat(value, token);
	}

	result = get(hash_table, (void *)symbol, &val);
	if (result != SUCCESS) {
		free(symbol);
		free(value);
		return result;
	}

	result = put(hash_table, (void *)symbol, (void *)value);
	if (result != SUCCESS) {
		free(symbol);
		free(value);
		return result;
	}

	if (val) {
		printf("Warnning: %s redefined.\n", symbol);
		free(val);
		free(symbol);
	}

	return SUCCESS;

}

int get_out_file_names(int argc, int *index, char **argv)
{
	if (argv[*index][2] != '\0') {
		strcpy(out_file_name, argv[*index] + 2);
	} else if (++*index < argc) {
		if (*index + 1 < argc) {
			printf("Error: Too many output files.\n");
			return INVALID_NR_PARAMS;
		}
		strcpy(out_file_name, argv[*index]);
	}

	return SUCCESS;
}

int parse_cmd_args(int argc, char **argv)
{
	int index = 1;
	int result;
	char *path;
	int len;

	while (index != argc) {
		if (argv[index][0] == '-' && argv[index][1] == 'D') {
			if (argv[index][2] == '\0')
				result = parse_arg(argv[++index]);
			else
				result = parse_arg(argv[index] + 2);

			if (result != SUCCESS) {
				fprintf(stderr, "Can not parse -D params.\n");
				return result;
			}

		} else if (argv[index][0] == '-' && argv[index][1] == 'I') {
			if (argv[index][2] == '\0')
				len = strlen(argv[index + 1]) + 1;
			else
				len = strlen(argv[index]) - 1;

			path = (char *)malloc(len * sizeof(char));
			if (!path)
				return -ENOMEM;

			if (argv[index][2] == '\0')
				strcpy(path, argv[++index]);
			else
				strcpy(path, argv[index] + 2);

			result = addItemToDoubleLinkedList(list, (void *)path);
			if (result != SUCCESS) {
				free(path);
				return result;
			}

		} else
			break;

		++index;
	}

	strcpy(in_file_name, STDIN);
	strcpy(out_file_name, STDOUT);
	if (index < argc) {
		if (argv[index][0] == '-' && argv[index][1] == 'o') {
			result = get_out_file_names(argc, &index, argv);
			if (result != SUCCESS)
				return result;

		} else {
			strcpy(in_file_name, argv[index]);
			++index;
			if (index < argc) {
				if (argv[index][0] != '-') {
					strcpy(out_file_name, argv[index]);
					if (index + 1 < argc) {
						printf("Error: Too many output "
							"files.\n");
						return INVALID_NR_PARAMS;
					}
				} else {
					result = get_out_file_names(argc,
						&index, argv);
					if (result != SUCCESS)
						return result;

				}
			}
		}
	}

	return SUCCESS;
}

void get_file_input_path(char *path, char *in_file_name, int len)
{
	if (strcmp(in_file_name, STDIN) == 0) {
		path[0] = '.';
		path[1] = '\0';
		return;
	}

	--len;
	while (len >= 0 && in_file_name[len] != '/') {
	// in_file_name[len] != '\\' && in_file_name[len] != '/') {
		--len;
	}

	if (len < 0) {
		path[0] = '.';
		path[1] = '\0';
	} else {
		strncpy(path, in_file_name, len + 1);
		path[len + 1] = '\0';
	}
}

void split_line(char *line, char words[][WORD_SIZE], int *nr_words, char *delm)
{
	char *ret;

	*nr_words = 0;

	ret = strtok(line, delm);
	if (!ret)
		return;

	while (ret) {
		strcpy(words[*nr_words], ret);
		++*nr_words;
		ret = strtok(NULL, delm);
	}
}

#endif // PARSE_H
