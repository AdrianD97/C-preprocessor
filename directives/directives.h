#ifndef DIRECTIVES_H
#define DIRECTIVES_H

int is_identifier(char *str)
{
	int res1, res2, res3, i = 1;

	res1 = (int)(str[0] - 'a');
	res2 = (int)(str[0] - 'A');
	if (str[0] != '_' && (res1 < 0 || res1 > 25)
		&& (res2 < 0 || res2 > 25)) {
		return 0;
	}

	while (str[i] != '\0') {
		res1 = (int)(str[i] - 'a');
		res2 = (int)(str[i] - 'A');
		res3 = (int)(str[i] - '0');

		if (str[i] != '_' && (res1 < 0 || res1 > 25) &&
			(res2 < 0 || res2 > 25) && (res3 < 0 || res3 > 9)) {
			return 0;
		}
		++i;
	}
	return 1;
}

int define_helper(char *word, char words[][WORD_SIZE],
	int nr_words, int *len, int i, int *is_multiline)
{
	int len_;

	for (; i < nr_words; ++i) {
		if (words[i][0] == '\\') {
			*is_multiline = 1;
			break;
		}

		len_ = strlen(words[i]);
		if (words[i][len_ - 1] == '\\') {
			words[i][len_ - 1] = '\0';
			--len_;
			*is_multiline = 1;
		}

		strcat(word, words[i]);
		*len += len_;
		strcat(word, " ");
		++*len;
	}

	return SUCCESS;
}

int define_directive(FILE *f_in, FILE *f_out,
	char words[][WORD_SIZE], int nr_words)
{
	char *symbol;
	char *value;
	int len = 1;
	int str_len, nr_words_def;
	char word[MAPPING_SIZE];
	char words_def[LINE_WORDS][WORD_SIZE];
	int result;
	char *line;
	int is_multiline;
	char *delm = "\t ";
	void *val;

	if (!is_identifier(words[1])) {
		printf("Error: Wrong identifier \'%s\'.\n", words[1]);
		return INCORRECT_IDENTIFIER;
	}

	word[0] = '\0';

	symbol = (char *)malloc((strlen(words[1]) + 1) * sizeof(char));
	if (!symbol) {
		printf("Error: Can not alloc memory"
			" for \'symbol\' variable.\n");
		return -ENOMEM;
	}
	strcpy(symbol, words[1]);

	if (nr_words >= 3) {
		is_multiline = 0;
		result = define_helper(word, words,
			nr_words, &len, 2, &is_multiline);
		if (result != SUCCESS) {
			free(symbol);
			return result;
		}

		if (is_multiline) {
			line = (char *)malloc(LINE_SIZE * sizeof(char));
			if (!line) {
				printf("Error: Can alloc memory"
				" for \'line\' variable.\n");
				return -ENOMEM;
			}

			// Multiline define
			while (fgets(line, LINE_SIZE, f_in) != NULL) {
				is_multiline = 0;
				str_len = strlen(line);

				if (line[str_len - 1] == '\n') {
					if (str_len > 1 &&
						line[str_len - 2] == '\r') {
						--str_len;
					}
					line[str_len - 1] = '\0';
				}

				split_line(line, words_def,
					&nr_words_def, delm);
				if (nr_words_def == 0) {
					fprintf(f_out, "\n");
					break;
				}

				result = define_helper(word, words_def,
					nr_words_def, &len, 0, &is_multiline);
				if (result != SUCCESS) {
					free(symbol);
					free(line);
					return result;
				}


				if (!is_multiline) {
					break;
				}
			}

			free(line);
		}
	}

	value = (char *)malloc(len * sizeof(char));
	if (!value) {
		printf("Error: Can not alloc memory for \'value\' variable.\n");
		free(symbol);
		return -ENOMEM;
	}

	strcpy(value, word);
	val = get(hash_table, (void *)symbol);

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

#endif // DIRECTIVES_H
